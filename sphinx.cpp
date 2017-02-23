#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include "uart.h"
#include "servo.h"
#include "ranger.h"
#include "cli.h"
#include "bluet.h"

extern int range;

#define SIMULATOR true

#define DAEMON 0


// global vars
int dontDie=true;
Servo *srvo[SRVO_NUM_SRVOS];
Arm *arm;

#if DAEMON
int demon=true; // being run as a daemon
#else
int demon=false; // being run as a daemon
#endif

void initSphinx(void)
{
  //  int i;

  arm=new Arm();

  // presume we are hanging vertically.
  srvo[SRVO_SHLDR]=new Servo(SRVO_SHLDR_HOME,SRVO_SHLDR_LOWER,SRVO_SHLDR_HOR,SRVO_SHLDR_UPPER,SRVO_SHLDR_MIN,SRVO_SHLDR_MAX,SRVO_SHLDR);
  delay(1000); // move the shoulder first and wait a bit.

  srvo[SRVO_ELBW]=new Servo(SRVO_ELBW_HOME,SRVO_ELBW_LOWER,SRVO_ELBW_HOR,SRVO_ELBW_UPPER,SRVO_ELBW_MIN,SRVO_ELBW_MAX,SRVO_ELBW);
  srvo[SRVO_BASE]=new Servo(SRVO_BASE_HOME,SRVO_BASE_HOME,SRVO_BASE_HOME,SRVO_BASE_HOME,SRVO_BASE_MIN,SRVO_BASE_MAX,SRVO_BASE);
  srvo[SRVO_WRST]=new Servo(SRVO_WRST_HOME,SRVO_WRST_LOWER,SRVO_WRST_HOR,SRVO_WRST_UPPER,SRVO_WRST_MIN,SRVO_WRST_MAX,SRVO_WRST);

  //  for (i=0;i<SRVO_NUM_SRVOS;i++)
  //    srvo[i]->goHome(FULL_SPEED);
}
 

static void handler(int signum)
{
  dontDie=false;
}

void * dataComm(void* threadid)
{
  // thread to listen to bluetooth
  bluet(); // 
  pthread_exit(NULL);
}

void * srvoTimer(void* threadid)
{
  // this is a timer thread that calls each servo object
  // if they are in the process of moving, this facilitates the
  // countdown
  int i;
  while(dontDie)
  {
    for (i=0;i<SRVO_NUM_SRVOS;i++)
      {
	srvo[i]->moveMe(); // and call him
	arm->update();
      }
    delay(SRVO_TIMER_TICK); // sleep this long before moving again - 50ms
  }
  pthread_exit(NULL);
}


void sim(void)
{
  /*  range+=3;
  if (range>MAX_RANGE)
    range=MIN_RANGE;
  */

  /*
  platAngle+=rotation;
  if (platAngle>=platToGo)
    rotation-=ROT_INC;
  if (platAngle<0)
    platAngle=ROT_INC;
  */
  /*
  if (srvoCurPos[0]<srvoTrgPos[0])
    srvoCurPos[0]+=5;
  else if (srvoCurPos[0]>srvoTrgPos[0])
    srvoCurPos[0]-=5;

  if (srvoCurPos[1]<srvoTrgPos[1])
    srvoCurPos[1]+=5;
  else if (srvoCurPos[3]>srvoTrgPos[1])
    srvoCurPos[1]-=5;

  if (srvoCurPos[2]<srvoTrgPos[2])
    srvoCurPos[2]+=5;
  else if (srvoCurPos[2]>srvoTrgPos[2])
    srvoCurPos[2]-=5;

  */
}




int wristAng(double x3,double y3,double x2,double y2)
{
  // compute the wrist angle based on the slope of the forearm
  // 
  double ang,opp,adj;;
  int wa;
  opp=y2-y3; // 
  adj=x2-x3;
  ang=opp/adj;
  ang=atan(ang);
  ang=ang* (float) 180/M_PI; //convert to degrees
  wa=ang;
  printf("wrist angle=%d\n",wa);
  return wa;
}

int calcAngle(double adj1,double adj2, double opp)
{
  // return the angle of a triangle, given the 3 lengths
  // Inv cos of angle= (adj Leg1^2 +adj leg2^2 - opp leg ^2)/2(adj1*adj1)
  double angA;
  int a;
  
  angA=(adj1*adj1)+(adj2*adj2)-(opp*opp);
  //  printf("angA= %f\n",angA);
  angA/=(float) (2*adj1*adj2);
  //  printf("angA= %f\n",angA);

  double aCOS=acos(angA);
  //  printf("aCos= %f\n",aCOS);
  angA=aCOS* (float) 180/M_PI;

  //printf("angle=%f\n",angA);
  a=angA;
  return a;
}
int triAngle(double A,double B, double C, int *a, int *b, int *c)
{
  // return the angles of a triangle, given the lengths
  // we can round the angles, but need double lengths
  //printf("A=%d, B=%d, C=%d\n",A,B,C);
  *c=calcAngle(A,B,C);
  *b=calcAngle(A,C,B);
  *a=calcAngle(B,C,A);
}

int triCalc(double x3,double y3, double *x2,double *y2,int *a, int *b, int *c)
{
  // for a given triangle whose lengths A, B are known
  // and whose origin point x1,y1 is known
  // take as input x3,y3 and calculage x2,y2,a,b,c

  // A=UPARM_LEN, B=FOREARM_LEN, X1=ARM_X_LEN, Y1=ARM_Y_LEN
  double A,B,C;
  double len;
  double x1,y1,x,y;
  x1=ARM_X_LEN; //12
  y1=ARM_Y_LEN; //16
  A=UPARM_LEN;
  B=FOREARM_LEN;
  // get the third leg
  // from Pythagorus - in a right triangle, X^2+Y^2=Z^2
  // take the x3,y3 from x1,y1, and solve for C
  x=x1-x3;
  y=y1-y3;
  len=(float) sqrt((x*x) + (y*y));
  //  printf("x=%f, y=%f, h=%f\n",x,y,len);
  
  C=len;
  // get the angles
  triAngle(A,B,C,a,b,c); // with the legs known, we can calc the angles
  // now find X2,Y2
  // to do that, we will use the soh rule twice.
  // first figure out the upper right triangle angle for X3,Y3, from X1,Y1
  // add the angle of the arm at that point.
  // That leaves the upper angle for the right triangle for X3,y3, X2,Y2
  // and solve.  Probably an easier way...

  double angle= (x1-x3)/ C;
  angle=asin(angle);
  angle*=(float) 180/M_PI;
  angle+=*a;
  angle=180-angle;
  // sin angle*hyp=x.  B=HYP=forearm=12
  angle*=(float)M_PI/180;
  *x2=x3+sin(angle)*B;
  *y2=y3-cos(angle)*B;
  printf("x1:%.2f, y1:%.2f, x2:%.2f, y2:%.2f, x3:%.2f, y3:%.2f, A:%.2f, B:%.2f ,C:%.2f,a:%d, b:%d, c%d\n",
	 x1,y1,*x2,*y2,x3,y3,A,B,C,*a,*b,*c);
}


int main(void) {
  WINDOW *win;


#if 0
  unsigned short i;
  /*
  extern unsigned short h2uPos[][9];
  unsigned short j,k,s,e,w;
  i=j=k=0;
  s=h2u[0][0];
  e=h2u[0][1];
  w=h2u[0][2];
  printf("s:%d, e:%d, w:%d\n",s,e,w);
  */
  printf("servo class size %d\n",sizeof(Servo));
  printf("servo class size %d\n",sizeof(short));
  printf("servo class size %d\n",sizeof(int));
  // calculate the arc and positions
  // first the arc positions.
  // max we can have the radius is: 
  // Dist from 0,0 to ARMX,ARMY - Const depth of phone holder
  // distance between two points, is pythagorean x^2+y^2=Dist^2


  double xc[16],yc[16],hyp[16],x2,y2,x,y;
  int angA[16],angB[16], angW[16]; // hard coded for now
  double A,B;
  int C,r,radius,h,angle,a,b,c;
  double rad,yd,hd,tan;
  x=ARM_X_LEN*ARM_X_LEN;
  y=ARM_Y_LEN*ARM_Y_LEN;

  printf("X1,Y1=Base, X2,Y2=Elbow, X3,Y3=Wrist\n");

  rad=x+y;
  rad=sqrt(rad);
  radius=r=rad-HOLDER_LEN; // this is the ideal radius from the center of the head
  printf("X: %d, y:%d, radius is %f,%d - holder: %d\n",ARM_X_LEN,ARM_Y_LEN,rad,r,HOLDER_LEN);
  // but due to mechanics, we have to reduce by 4 inches more
  printf("With correction\n");
  radius=r=r-4;

  printf("x1: %d, y1:%d, radius is %f,%d - holder: %d\n\n",ARM_X_LEN,ARM_Y_LEN,rad,r,HOLDER_LEN);


  r*=r;  // square it
  //  for (i=0;i<radius;i++)
  for (i=0;i<=12;i++)
    {
      // ID wrist locations = radius points
      // for each point x, calulate y, based on the radius.
      // x^2 + y^2 = r^2
      xc[i]=i;
      x=i*i; // square it
      yd=sqrt((double)r-x);
      y=yd;
      yc[i]=y;
      // now fill out the triangle
      triCalc(xc[i],yc[i],&x2,&y2,&a,&b,&c);
      angW[i]=wristAng(xc[i],yc[i],x2,y2);
    }

  


#if 0
  // now figure out the angle of the wrist, at each location.  
  printf ("tangent\n");
  for (i=0;i<16;i++)
    {
      tan=(float)yc[i]/xc[i];
      printf("%d: %d,%d  - angle: %f\t",xc[i],yc[i],tan);
      tan=atan(tan);
      tan=(float) 180/M_PI *tan;
      printf("%d - %f\t\n",tan);
    }

  // now figure out hypotenuse from robot arm,  of all positions on arc
  // again, pythagorean theorm x^2 + y^2 = hyp^2
  // location of arm root is: ARM_X_LEN,ARM_Y_LEN
  // so the math is (ARM_X_LEN-x)^2 + (ARM_Y_LEN-y)^2=hyp^2
  // 
  printf("Hypotenuse from arm\n");
  printf("x,\ty,\tHyp\n");
  for (i=0;i<16;i++)
    {
      // for each point x,y calulate hypotenuse from the arm base
      x=ARM_X_LEN-xc[i];
      y=ARM_Y_LEN-yc[i];
      printf("%d\t%d",xc[i],yc[i]);
      x*=x;y*=y;
      hd=sqrt(x+y);
      h=hd;
      hyp[i]=h;
      printf("\t%f\n",hd);
    }
  // now we have the length of the hypotenuse for all of the locations on the arc
  // calculate the angle of the shoulder A, and elbow B, for each locattion
  // sum of angles a,b,c in a triangle of len A,B,C equals 180
  // Inv cos of angle= (adj Leg^2 +adj leg^2 - opp leg ^2)/2(adj*adj)
  // upper arm=A=UPARM_LEN.  forearm=B=FOREARM_LEN.  
  printf("Angles of servos\n");
  printf("a\tb\tC\n");
  for (i=0;i<16;i++)
    {
      A=(hyp[i]*hyp[i])+ (float)(UPARM_LEN*UPARM_LEN) - (float)(FOREARM_LEN*FOREARM_LEN);
      printf("%i - Num %f ",i,A);
      float den=(float)(2*UPARM_LEN*hyp[i]);
      A/=den;
      printf("/%f=%f - ",den,A);
      float COS=acos(A);
      A=(float) 180/M_PI *COS;
      printf("Cos: %f",A);
      angA[i]=A;

      B=(UPARM_LEN*UPARM_LEN) + (FOREARM_LEN*FOREARM_LEN) - (hyp[i]*hyp[i]);
      B/=(float)(2*UPARM_LEN*FOREARM_LEN);
      B=acos(B);
      B*=180/M_PI;
      angB[i]=B;
      
      C=180-(A+B);
      
      printf(" A: %d B: %d C: %d\n",angA[i],angB[i],C);
    }
#endif

  // calibration positions
  //  printf("Calibration positions\n");
  //  printf("Pos:\tBase\t shlder\telbow\twrist\n");
  //  for (i=0;i<NUM_ARM_POS;i++)
  //    printf("%2d:\t%5d,\t%5d,\t%5d,\t%5d\n",i,base[i],shoulder[i],elbow[i],wrist[i]);
  exit(0);
#endif
  
  initGPIO();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  initUart();
  initSphinx();

  resetBTimeout(); // set the BT watchdog timer

  if (!demon)
    {
      if ((win=initscr())==NULL)
	{
	  printf("Error intializing curses\n");
	  exit(1);
	}
      curs_set(0);
      mvwaddstr(win,1,33,"Sphinx");
      noecho();
      nodelay(win,1); // make getch nonblocking
    }
  //  printf("handler\n");
  // threads
  pthread_t threads[2];
  int rc;
  long t=0;

  // setup sig handler
  struct sigaction newAct,oldAct;
  newAct.sa_handler=handler;
  sigemptyset(&newAct.sa_mask); // clear
  newAct.sa_flags=0; // clear
  sigaction(SIGUSR1,NULL,&oldAct); // save it
  sigaction(SIGUSR1,&newAct,NULL);

  //  printf ("starting threads\n");
  
  rc=pthread_create(&threads[0],NULL,ranger,(void *) t);
  rc=pthread_create(&threads[1],NULL,dataComm,(void *) t);
  rc=pthread_create(&threads[2],NULL,srvoTimer,(void *) t);
  // parent
  if (!demon)
    mon(win);
  else
    while (1)
      sleep(100);
      
  // need to kill the child process
  pthread_kill(threads[0],SIGUSR1);
  //pthread_kill(threads[1],SIGUSR1);
  // sent the die msg.  Now wait for him to die
  pthread_join(threads[0],NULL);
  //printf("ranger dead\n");
  pthread_join(threads[1],NULL);
  //printf("datacomm dead\n");
  pthread_join(threads[2],NULL);
  //printf("srvo dead\n");
  if (!demon)
    {
      clear();
      curs_set(1);
      refresh();
      endwin();
    }
  closeUart();
  digitalWrite(LED_PIN, LOW); // turn off led
  pinMode(LED_PIN, INPUT); // reset it to input
  return 0;
}

