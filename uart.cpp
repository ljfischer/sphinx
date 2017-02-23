#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
// for uart
#include <fcntl.h>
#include <termios.h>

#include "uart.h"

int uartFS=-1;

void initUart(void)
  {
    uartFS=open("/dev/ttyAMA0",O_RDWR|O_NOCTTY|O_NDELAY);
    // config
    struct termios opt;
    tcgetattr(uartFS,&opt);
    opt.c_cflag=B115200|CS8|CLOCAL|CREAD;
    //    opt.c_cflag=B9600|CS8|CLOCAL|CREAD;
    opt.c_iflag=IGNPAR;
    opt.c_oflag=0;
    opt.c_lflag=0;
    tcflush(uartFS,TCIFLUSH);
    tcsetattr(uartFS,TCSANOW,&opt);
  }

  void closeUart(void)
  {
    close(uartFS);
  }

  void txUartNull(unsigned char *txBuf)
  {
    // tx a buf out the uart.  If len==0, then it is null terminated.
    if (uartFS!=-1)
      {
	int len;
	len=strlen((const char *) txBuf);
	write(uartFS,txBuf,len);
      }
  }

  void txUart(unsigned char *txBuf,int len)
  {
    // tx a buf out the uart.  If len==0, then it is null terminated.
    if (uartFS!=-1)
      {
	if (len==0)
	  len=strlen((const char *) txBuf);
	len=write(uartFS,txBuf,len);
      }
  }

#define RX_BUF_SIZE 32
  int chkUart(unsigned char *rxBuf,int rxBufSize)
  {
    int len =-1;
    if (uartFS!= -1)
      {
	len=read(uartFS,(void*)rxBuf,rxBufSize-1);
	if (len<0)
	  ; // error
	else if (len==0)
	  ; //no data
	else
	  {
	    // data in the buf
	    rxBuf[len]=0; // null  terminate;
	  }
      }
    return len;
  }
