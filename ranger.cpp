#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <signal.h>
#include <pthread.h>
// for uart
#include <fcntl.h>
#include <termios.h>

#include "uart.h"
#include "servo.h"
#include "ranger.h"

extern int dontDie;

int range;

void initGPIO(void) {
  wiringPiSetup();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
 
  //TRIG pin must start LOW
  digitalWrite(TRIG_PIN, LOW);
  delay(30); // 30 ms
}
 
int getCM(void) {
  //Send trig pulse
  //        printf("Ping\n");
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(50);
  digitalWrite(TRIG_PIN, LOW);
 
  //Wait for echo start
  while(digitalRead(ECHO_PIN) == LOW && dontDie)
    {
      //	    printf("waiting for echo to start\n");
      ;
    }
 
  //Wait for echo end
  long startTime = micros();
  while(digitalRead(ECHO_PIN) == HIGH && dontDie)
    {
      //	    printf("waiting for echo to end\n");
      ;
    }
  long travelTime = micros() - startTime;
 
  //Get distance in cm
  int distance = travelTime / 58;
 
  return distance;
}
void * ranger(void* threadid)
{
  // thread to perform ranging
  while (dontDie)
    {
      range=getCM();
      delay(100);
    }
  pthread_exit(NULL);
}
