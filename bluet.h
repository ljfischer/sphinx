#ifndef Bluet_h
#define Bluet_h
void bluet(void);
void resetBTimeout(void);

extern int btConn;
extern unsigned char blueErr;
extern unsigned long helloCnt,cmdCnt,errCnt;
extern  struct timeval btDataTime;
#define CMD_CHAR '$'
#define HELLO_CHAR '#'
#define BT_CONN_TIMEOUT 100 // if no ping in 10 seconds, tear down the connection

#define LED_PIN 6

#endif
