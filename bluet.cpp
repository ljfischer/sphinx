#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include "bluet.h"
#include "servo.h"

extern int dontDie;
extern int demon;

int btConn=false;
unsigned long helloCnt=0,cmdCnt=0,errCnt=0;
unsigned char blueErr;

struct timeval btDataTime;
struct timezone btDataTz;

void btConnected(int val)
{
  btConn=val;
}


void resetBTimeout(void)
{
  // reset the time that we last received data over the BT conn
  gettimeofday(&btDataTime,&btDataTz);
}

void bluet(void)
{
  char address[18]="00:1B:DC:06:D2:2A"; // BT MAC Addr
  int sel;

  fd_set rFlag,wFlag;
  struct timeval waitd;

  struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
  char buf[100] = { 0 };
  int s, client=-1;// bytes_read;
  socklen_t opt = sizeof(rem_addr);

  // allocate socket
  s = socket(AF_BLUETOOTH, SOCK_STREAM|SOCK_NONBLOCK, BTPROTO_RFCOMM);


  // bind socket to port 1 of the first available 


  // local bluetooth adapter
  loc_addr.rc_family = AF_BLUETOOTH;
  //    loc_addr.rc_bdaddr = *BDADDR_ANY;
  str2ba(address, &loc_addr.rc_bdaddr);
  loc_addr.rc_channel = (uint8_t) 1;
  bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
  
  while (dontDie)
    {
      //      printf("No conn\n");

      btConn=false;
      digitalWrite(LED_PIN, LOW);
      //      btConnected(false);
      // put socket into listening mode
      listen(s, 1);

      while (dontDie && client<0)
	{
	  // accept one connection
	  client = accept(s, (struct sockaddr *)&rem_addr, &opt);
	  delay(100); // wait a bit
	}

      int flags=fcntl(client,F_GETFL,0);
      fcntl(client,F_SETFL,flags|O_NONBLOCK);

      ba2str( &rem_addr.rc_bdaddr, buf );
      //      fprintf(stderr, "accepted connection from %s\n", buf);
      memset(buf, 0, sizeof(buf));

      // read data from the client
      btConn=true;
      digitalWrite(LED_PIN, HIGH);

      //      btConnected(true);
      //      printf("connect\n");
      resetBTimeout(); // conn established

  while (dontDie && btConn)
	{
	  int i;

	  waitd.tv_sec=1;
	  FD_ZERO(&rFlag);
	  FD_ZERO(&wFlag);
	  FD_SET(client,&rFlag);

	  sel=select(client+1,&rFlag,&wFlag,(fd_set*)0,&waitd);
	  if (sel<0)
	    continue;
	  else if (sel==0)
	    {
	      struct timeval curTime;
	      struct timezone tz;
	      gettimeofday(&curTime,&tz);
	      if (curTime.tv_sec - btDataTime.tv_sec > BT_CONN_TIMEOUT)
		{
		  // havent heard anything in a while.
		  // tear down the connection and listen
		  btConn=false;
		  //		  btConnected(false);
		}
	    }
	  // socket has data for reading
	  if (FD_ISSET(client,&rFlag))
	    {
	      FD_CLR(client,&rFlag);
	      if ((i=recv(client,buf,sizeof(buf),0))<0)
		{
		  //		  printf("err - %d, errno %d\n",i,errno);
		  //		  perror("recv");
		  btConn=false;
		  //		  btConnected(false);
		  break;
		}
	      else
		{
		  // received data
		  //		  printf ("char\n");
		  resetBTimeout();
		  buf[i]=0; // terminate it
		  //		  printf("New received [%s]\n", buf);
		  if (buf[0]==CMD_CHAR)
		    {
		      cmdCnt++;
		      unsigned char cmd=buf[1];
		      //send a message to execute this command

		      switch (cmd)
			{
			case '0':
			  //printf("Park\n");
			  cmdPark();
			  break;
			case '1':
			  //printf("Hor\n");
			  cmdHor();
			  break;
			case '2':
			  //printf("Up\n");
			  cmdUp();
			  break;
			case '3':
			  //printf("Down\n");
			  cmdDown();
			  break;
			case '4':
			  //printf("CCW\n");
			  cmdCCW();
			  break;
			case '5':
			  //printf("CW\n");
			  cmdCW();
			  break;
			case '6':
			  //printf("Calibrate\n");
			  calibrate();
			  break;
			case '7':
			  //printf("sculpt\n");
			  sculpt();
			  break;
			case '8':
			  //printf("home\n");
			  gotoPos(0); // home
			  break;
			default:
			  blueErr=buf[1];
			  errCnt++;
			}
		    }
		  else if (buf[0]=HELLO_CHAR)
		    {
		      helloCnt++;
		    }
		  else
		    {
		      errCnt++;
		    }
		}

	      /*
		bytes_read = read(client, buf, sizeof(buf));
		if( bytes_read > 0 ) {
		printf("received [%s]\n", buf);
	      */
	    }
	}

      // close connection
      close(client);
      client=-1;
    }
  close(s);
}
