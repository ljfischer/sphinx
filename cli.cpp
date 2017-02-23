#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
// for uart
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>

#include "uart.h"
#include "servo.h"
#include "ranger.h"
#include "cli.h"
#include "bluet.h"

extern int range;
extern Servo *srvo[];
extern Arm *arm;
unsigned short cliSrvo=0;


void updateStatic(WINDOW *win)
{
  // print out all of the static text
  mvwaddstr(win,2,1,"Range:");

  mvwaddstr(win,3,1,"Base Servo:");
  mvwaddstr(win,3,30,"Target: ");
  mvwaddstr(win,3,50,"Min: ");
  mvwaddstr(win,3,65,"Max: ");

  mvwaddstr(win,4,1,"Shldr Servo:");
  mvwaddstr(win,4,30,"Target: ");
  mvwaddstr(win,4,50,"Min: ");
  mvwaddstr(win,4,65,"Max: ");

  mvwaddstr(win,5,1,"Elbow Servo:");
  mvwaddstr(win,5,30,"Target: ");
  mvwaddstr(win,5,50,"Min: ");
  mvwaddstr(win,5,65,"Max: ");

  mvwaddstr(win,6,1,"Wrist Servo:");
  mvwaddstr(win,6,30,"Target: ");
  mvwaddstr(win,6,50,"Min: ");
  mvwaddstr(win,6,65,"Max: ");

  mvwaddstr(win,7,1,"Target Servo:");

  mvwaddstr(win,8,1,"BlueTooth:");
  mvwaddstr(win,8,30,"BT Time:");

  mvwaddstr(win,9,1,"# Hello: ");
  mvwaddstr(win,9,30,"# Cmd: ");
  mvwaddstr(win,9,50,"# Error: ");

  mvwaddstr(win,10,1,"Seq: ");
  mvwaddstr(win,10,30,"Cntr: ");

  //  mvwaddstr(win,10,50,"S Seq: ");

  mvwaddstr(win,11,1,"E-Stop: ");
}
void updateDynamic(WINDOW *win)
{
  // print out the dynamic text
  char buf[50];
  unsigned short i; 

  sprintf(buf,"%d cm   ",range);
  mvwaddstr(win, 2,15,buf);

  /*
  sprintf(buf,"%d Deg   ",platAngle);
  mvwaddstr(win, 3,15,buf);
  sprintf(buf,"%d Deg   ",platGoTo);
  mvwaddstr(win, 3,40,buf);
  */
  

  for (i=0;i<SRVO_NUM_SRVOS;i++)
    {
      sprintf(buf,"%d ms  ",srvo[i]->getPos());
      mvwaddstr(win, 3+i,15,buf);
      sprintf(buf,"%d ms  ",srvo[i]->getTrg());
      mvwaddstr(win, 3+i,40,buf);
      sprintf(buf,"%d ms  ",srvo[i]->getMin());
      mvwaddstr(win, 3+i,55,buf);
      sprintf(buf,"%d ms  ",srvo[i]->getMax());
      mvwaddstr(win, 3+i,70,buf);
    }
  sprintf(buf,"%d ",cliSrvo);
  mvwaddstr(win, 7,15,buf);
  sprintf(buf,"%d ",SERVO_CLI_INC);
  mvwaddstr(win, 7,20,buf);

  mvwaddstr(win, 8,15,btConn?"Connected":"No Conn   ");

  struct timeval curTime;
  struct timezone tz;
  gettimeofday(&curTime,&tz);
  
  sprintf(buf,"%d           ",curTime.tv_sec-btDataTime.tv_sec);
  mvwaddstr(win, 8,40,buf);

  sprintf(buf,"%d   ",helloCnt);
  mvwaddstr(win, 9,20,buf);
  sprintf(buf,"%d   ",cmdCnt);
  mvwaddstr(win, 9,40,buf);
  sprintf(buf,"%d - %d ",errCnt,blueErr);
  mvwaddstr(win, 9,70,buf);

  sprintf(buf,"%d   ",arm->seqNum);
  mvwaddstr(win, 10,20,buf);
  sprintf(buf,"%d   ",arm->cntr);
  mvwaddstr(win, 10,40,buf);

  //  sprintf(buf,"%d   ",arm->seqNum!=-1 ? seqPos[arm->seqNum][SRVO_SHLDR]:0);
  //  mvwaddstr(win, 10,70,buf);

  sprintf(buf,"%d   ",eStop);
  mvwaddstr(win, 11,15,buf);
}
unsigned short getNum(WINDOW *win,unsigned short min, unsigned short max,int row,int col)
{
  // let the user enter digits only

  unsigned short i,num=0,ret=-1, done=false;
  char buf[80];

  nodelay(win,false);
  echo();
  move(row,col);
  clrtoeol();
  while (!done)
    {
      i=getstr(buf);
      if (i!=ERR)
	{
	  num=atoi(buf);
	  if (num<min || num>max)
	    {
	      beep();
	      move(row,col);
	      clrtoeol();
	      
	    }
	  else
	    {
	      ret=num;
	      done=true;
	    }
	}	  
    }
  noecho();
  nodelay(win,true);
  return ret;
}
unsigned char crStr=13;


void parseCmd(WINDOW *win, unsigned char ch)
{
  // parse the char and act on it
  unsigned short i,srv;
  switch (ch)
    {
    case '0':
    case '1':
    case '2':
    case '3':
      cliSrvo=ch-'0'; // make it a decimal
      break;

    case '+':
      srvo[cliSrvo]->gotoMS(srvo[cliSrvo]->getPos()+SERVO_CLI_INC,HALF_SPEED,true);
      break;
    case '-':
      srvo[cliSrvo]->gotoMS(srvo[cliSrvo]->getPos()-SERVO_CLI_INC,HALF_SPEED,true);
      break;

    case 'P': 
    case 'p':
      // park - go to park position
      cmdPark();
      break;
    case 'U':
      // Upper - go to upper position
      arm->h2HalfUp(); //vert
      break;
    case 'u': 
      // Upper - go to upper position
      arm->halfUp2H(); // back to hor
      break;
    case 'L':
      // - go to lower position
      arm->h2HalfLow(); //vert
      break;
    case 'l': 
      // - go to lower position
      arm->halfLow2H(); //vert
      break;
    case 'H': 
    case 'h':
      //  - go to horizontal position
      cmdHor();
      break;
      
    case 'a': 
    case 'A':
      // absolute position of servos
      mvwaddstr(win,CMD_ROW,1,"Servo #: ");
      clrtoeol();
      i=getNum(win,0,SRVO_NUM_SRVOS-1,CMD_ROW,CMD_COL);
      if (i!= -1)
	{
	  srv=i;
	  mvwaddstr(win,CMD_ROW,CMD_COL+5,"Target ms: ");
	  refresh();
	  i=getNum(win,srvo[srv]->getMin(),srvo[srv]->getMax(),
		   CMD_ROW,CMD_COL+20);
	  if (i!=-1)
	    srvo[srv]->gotoMS(i,THREEQTR_SPEED,true);
	}
      break;
    case 'r':
    case 'R':
      // Ranger
      mvwaddstr(win,CMD_ROW,1,"Ranger Echo: ");
      clrtoeol();
      i=getNum(win,MIN_RANGE,MAX_RANGE,CMD_ROW,CMD_COL);
      if (i!= -1)
	range=i;
      break;
    case 'g':
    case 'G':
      // Goto Pos
      // send ALL servos to the predefined position
      mvwaddstr(win,CMD_ROW,1,"Position: ");
      clrtoeol();
      i=getNum(win,ARM_POS_0,NUM_ARM_POS-1,CMD_ROW,CMD_COL);
      if (i!= -1)
	{
	  //char buf[40];
	  //	  sprintf(buf,"B:%d S:%d E:%d W:%d   ",base[i],shoulder[i],elbow[i],wrist[i]);
	  //	  mvwaddstr(win, CMD_ROW,40,buf);
#if 1
	  //	  for (unsigned short j=0;j<SRVO_NUM_SRVOS;j++)
	  //	    srvo[j]->gotoPos(i);
	  gotoPos(i);
#else
	  srvo[SRVO_BASE]->gotoMS(base[i],bSpd[i],true);
	  srvo[SRVO_SHLDR]->gotoMS(shoulder[i],sSpd[i],true);
	  srvo[SRVO_ELBW]->gotoMS(elbow[i],eSpd[i],true);
	  srvo[SRVO_WRST]->gotoMS(wrist[i],wSpd[i],true);
#endif
	}
      break;
    case 'z':
    case 'Z':
      // hang in sleeping position.  Dont rotate
      for (int i=1;i<SRVO_NUM_SRVOS;i++)
	srvo[i]->goHome(HALF_SPEED);
      break;
    case 'c':
    case 'C':
      //    Run thru the calibration sequence
      calibrate();
      break;
    case 's':
    case 'S':
      //    sculpt
      sculpt();
      break;
    case '^':
      //    hor2Up via seq
      arm->h2Up();
      break;
    case 'v':
    case 'V':
      //    hor2Low via seq
      arm->h2Low();
      break;
    case '[':
      //    hor2Up via seq
      arm->up2H();
      break;
    case '<':
      //    hor2Up via seq
      cmdCCW();
      break;
    case '>':
      //    hor2Up via seq
      cmdCW();
      break;
    case ']':
      //    hor2Up via seq
      arm->low2H();
      break;
    case ' ':
      eStop=!eStop;
      break;
    }
}

void mon(WINDOW *win)
{
  unsigned short done=false;
  unsigned char ch;
  
  while (!done)
    {
      ch=getch();
      if (ch=='q'||ch=='Q')
	{
	  cmdPark();
	  delay(3000);
	  done=true;
	}
      else
	{
	  if (ch!=ERR)
	    {
	      parseCmd(win, ch);
	      updateStatic(win);
	      updateDynamic(win);
	      refresh();
	    }
	  delay(300);
#if SIMULATOR
	    sim();
#endif
	}
	  
    }
}
