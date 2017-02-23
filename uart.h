#ifndef Uart_h
#define Uart_h
  void initUart(void);
  void closeUart(void);
  void txUartNull(unsigned char *txBuf);
  void txUart(unsigned char *txBuf,int len);

#endif // Uart_h
