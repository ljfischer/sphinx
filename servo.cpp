#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <wiringPi.h>
#include "uart.h"
#include "servo.h"

extern Servo *srvo[];// ack - this is silly to do.
extern Arm *arm;

#define FAST_ROTATE 0

// pre-define the positions for the initial calibration
// 0, 135H1, 135V, 135H2, 225H1, 225V, 225H2, 450, 360

unsigned short base[]={SRVO_BASE_HOME,SRVO_BASE_135,SRVO_BASE_135,SRVO_BASE_135,SRVO_BASE_225,SRVO_BASE_225,SRVO_BASE_225,SRVO_BASE_450,SRVO_BASE_360};

unsigned short shoulder[]={SRVO_SHLDR_HOR,SRVO_SHLDR_135H,SRVO_SHLDR_135V,SRVO_SHLDR_135H,SRVO_SHLDR_225H,SRVO_SHLDR_225V,SRVO_SHLDR_225H,SRVO_SHLDR_450H,SRVO_SHLDR_360H};

unsigned short elbow[]={SRVO_ELBW_HOR,SRVO_ELBW_135H,SRVO_ELBW_135V,SRVO_ELBW_135H,SRVO_ELBW_225H,SRVO_ELBW_225V,SRVO_ELBW_225H,SRVO_ELBW_450H,SRVO_ELBW_360H};

unsigned short wrist[]={SRVO_WRST_HOR,SRVO_WRST_135H,SRVO_WRST_135V,SRVO_WRST_135H,SRVO_WRST_225H,SRVO_WRST_225V,SRVO_WRST_225H,SRVO_WRST_450H,SRVO_WRST_360H};

// and the speed with which we get there.
//unsigned short bSpd[]={HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED};

#if FAST_ROTATE
unsigned short bSpd[]={3,2,1,1,2,1,1,2,2};
#else
unsigned short bSpd[]={3,1,1,1,1,1,1,1,2};
#endif

unsigned short sSpd[]={HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED};

unsigned short eSpd[]={HALF_SPEED,HALF_SPEED,FULL_SPEED,HALF_SPEED,HALF_SPEED,FULL_SPEED,HALF_SPEED,HALF_SPEED,HALF_SPEED};
//unsigned short wSpd[]={FULL_SPEED,FULL_SPEED,FULL_SPEED,FULL_SPEED,FULL_SPEED,FULL_SPEED,FULL_SPEED,FULL_SPEED,FULL_SPEED};
unsigned short wSpd[]={FULL_SPEED,FULL_SPEED,6,5,FULL_SPEED,6,5,FULL_SPEED,FULL_SPEED};


#if FAST_ROTATE
unsigned short calTime[]={3,10,6,7,8,6,7,16,9}; // at hor speed 2
#else
unsigned short calTime[]={3,27,6,7,20,6,7,40,9}; // at hor speed 1
#endif

// microstep predef arrays - 9 sequences long
// no magic to the count of sequences.  Can be longer or shorter
unsigned short seqPosH2HalfU[][3]=
  {
    {1050,1550,1690}, // 0 H2U
    {1000,1530,1650},
    {970,1450,1630},
    {930,1380,1590},
    {930,1380,1560},
    {930,1300,1470},
    {0,0,0} // this lets usknow we are done
  };
unsigned short seqPosHalfU2H[][3]=
  {
    {940,1430,1600},
    {950,1480,1640},
    {1000,1530,1690},
    {1050,1550,1690},
    {0,0,0} // this lets usknow we are done
  };
unsigned short seqPosH2U[][3]=
  {
    {1050,1550,1690}, // 0 H2U
    {1000,1530,1690},
    {930,1480,1640},
    {930,1430,1650},
    {930,1330,1600},
    {930,1300,1550},
    {930,1280,1500},
    {930,1250,1450},
    {930,1230,1400},
    {930,1230,1350},
    {930,1200,1300},
    {980,1180,1270},
    {0,0,0} // this lets usknow we are done
  };
unsigned short seqPosU2H[][3]=
  {
    {980,1180,1300}, //9 U2H
    {930,1230,1350},
    {930,1280,1400},
    {930,1330,1450},
    {930,1330,1500},
    {930,1330,1540},
    {940,1430,1600},
    {950,1480,1640},
    {1000,1530,1690},
    {1050,1550,1690},
    {0,0,0} // this lets usknow we are done
  };
unsigned short seqPosH2L[][3]=
  {
    {1050,1550,1720}, // 18 H2L
    {1100,1575,1740},
    {1100,1600,1750},
    {1150,1650,1770},
    {1200,1730,1780},
    {1250,1780,1780},
    {1270,1760,1790},
    {1270,1760,1800},
    {1270,1760,1800},
    {0,0,0} // this lets usknow we are done
  };
unsigned short seqPosL2H[][3]=
  {
    {1270,1760,1820},
    {1250,1780,1800},
    {1240,1750,1790},
    {1220,1730,1780},
    {1200,1700,1780},
    {1180,1680,1750},
    {1150,1650,1740},
    {1100,1600,1720},
    {1050,1550,1690}, // 27 L2H
    {0,0,0} // this lets usknow we are done
  };
unsigned short seqPosH2HalfL[][3]=
  {
    {1050,1550,1720}, // 18 H2L
    {1100,1575,1740},
    {1100,1600,1750},
    {1150,1650,1770},
    {1200,1700,1780},
    {0,0,0} // this lets usknow we are done
  };
unsigned short seqPosHalfL2H[][3]=
  {
    {1200,1700,1780},
    {1180,1680,1750},
    {1150,1650,1740},
    {1100,1600,1720},
    {1050,1550,1690}, // 27 L2H
    {0,0,0} // this lets usknow we are done
  };

////////////////
// Speeds
////////////////

unsigned short seqSpdH2HalfU[][3]=
  {
    {2,2,2}, // H2U
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,1},
    {2,2,2},
    {0,0,0}
  };
unsigned short seqSpdHalfU2H[][3]=
  {
    {2,1,1},
    {2,1,1},
    {2,2,2},
    {2,2,2},
    {0,0,0}
  };

unsigned short seqSpdH2U[][3]=
  {
    {2,2,2}, // H2U
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,3},
    {2,2,3},
    {2,2,4},
    {0,0,0}
  };
unsigned short seqSpdU2H[][3]=
  {
    {2,2,2}, // U2H
    {2,1,1},
    {2,1,2},
    {2,1,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {0,0,0}
  };

unsigned short seqSpdH2L[][3]=
  {
    {2,2,1}, // H2L
    {2,2,1},
    {2,2,2},
    {2,2,1},
    {2,2,1},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {2,2,2},
    {0,0,0}
  };
unsigned short seqSpdL2H[][3]=
  {
    {2,2,1}, // L2H
    {2,2,1},
    {2,2,2},
    {2,2,1},
    {2,2,1},
    {2,2,1},
    {2,2,1},
    {2,2,2},
    {2,2,1},
    {0,0,0}
  };

unsigned short seqSpdH2HalfL[][3]=
  {
    {2,2,1}, // H2L
    {2,2,1},
    {2,2,2},
    {2,2,1},
    {2,2,1},
    {0,0,0}
  };
unsigned short seqSpdHalfL2H[][3]=
  {
    {2,2,1}, //
    {2,2,1},
    {2,2,1},
    {2,2,2},
    {2,2,1},
    {0,0,0}
  };

short eStop=false;
#define FULL_UP 1

void sculpt(void)
{
  // presume horizontal
  
#if 1


#if FULL_UP
  cmdUp();
  delay(VER_DELAY); // wait a bit
#else
  arm->h2HalfUp(); //vert
  delay(calTime[2]*1000);
#endif
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

  /*
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  */

#if FULL_UP
  cmdUp2Hor();
  delay(VER_DELAY); // wait a bit
#else
  arm->halfUp2H(); // back to hor
  delay(calTime[3]*1000);
#endif

  //  cmdDown();
  //  delay(VER_DELAY); // wait a bit
  // horizontal
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  // front and center

#if FULL_UP
  cmdUp();
  delay(VER_DELAY); // wait a bit
#else
  arm->h2HalfUp(); //vert
  delay(calTime[2]*1000);
#endif

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

#if FULL_UP
  cmdUp2Hor();
  delay(VER_DELAY); // wait a bit
#else
  arm->halfUp2H(); // back to hor
  delay(calTime[3]*1000);
#endif
//  cmdLow2Hor();
//  delay(VER_DELAY); // wait a bit

//  cmdUp();
//  delay(VER_DELAY); // wait a bit

#if 0
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdUp2Hor();
  delay(VER_DELAY); // wait a bit
#endif
  

  //  cmdDown();
  //  delay(VER_DELAY); // wait a bit

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

#if 0
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

    cmdDown();
    delay(VER_DELAY); // wait a bit
  // low ccw

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

    cmdLow2Hor(); 
    delay(VER_DELAY); // wait a bit
    // hor
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  // front and center
    cmdDown();
    delay(VER_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit
  cmdCW();
  delay(HOR_DELAY); // wait a bit

    cmdLow2Hor(); 
    delay(VER_DELAY); // wait a bit

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit

  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
  cmdCCW();
  delay(HOR_DELAY); // wait a bit
#endif

#endif
  // calibration complete.  Now sculpt
  //      delay(15000); // wait a bit
}
void calibrate(void)
{
#if 0
  for (int i=0;i<NUM_ARM_POS;i++)
    {
      gotoPos(i);
      delay(calTime[i]*1000);
    }
#else
  gotoPos(0); // horizontal
  delay(calTime[0]*1000);

  gotoPos(1); // first round position
  delay(calTime[1]*1000);

  arm->h2HalfUp(); //vert
  delay(calTime[2]*1000);

  arm->halfUp2H(); // back to hor
  delay(calTime[3]*1000);

  gotoPos(4); // slide sideways
  delay(calTime[4]*1000);

  arm->h2HalfUp(); // vert
  delay(calTime[5]*1000);

  arm->halfUp2H(); // back to hor
  delay(calTime[6]*1000);

  gotoPos(7); // slide sideways
  delay(calTime[7]*1000);

  gotoPos(8); // back to front
  delay(calTime[8]*1000);
#endif
  
#if 0
  delay(10000); // wait a bit
  // calibration complete.  Now sculpt
  sculpt();

  gotoPos(0); // and unwind it
  delay(15000); // wait a bit
#endif
}  

int getPosMS(unsigned short srvo,unsigned short pos)
{
  unsigned short ms=-1;
  if (srvo>=0 && srvo<SRVO_NUM_SRVOS && pos>=0 && pos<NUM_ARM_POS)
    {
      switch(srvo)
	{
	case SRVO_BASE:
	  ms=base[pos];
	  break;
	case SRVO_SHLDR:
	  ms=shoulder[pos];
	  break;
	case SRVO_ELBW:
	  ms=elbow[pos];
	  break;
	case SRVO_WRST:
	  ms=wrist[pos];
	  break;
	default:
	  printf("ERROR in MYID %d\n",srvo);
	}
    }
  return ms;
}
unsigned short getPosSpd(unsigned short srvo,unsigned short pos)
{
  unsigned short spd=-1;
  if (srvo>=0 && srvo<SRVO_NUM_SRVOS && pos>=0 && pos<NUM_ARM_POS)
    {
      switch(srvo)
	{
	case SRVO_BASE:
	  spd=bSpd[pos];
	  break;
	case SRVO_SHLDR:
	  spd=sSpd[pos];
	  break;
	case SRVO_ELBW:
	  spd=eSpd[pos];
	  break;
	case SRVO_WRST:
	  spd=wSpd[pos];
	  break;
	default:
	  printf("ERROR in MYID %d\n",srvo);
	}
    }
  return spd;
}

void gotoPos(unsigned short id)
{
  // moves all servo to the appropriate location, at speed
  // this is specifically for the calibration positions
  for (unsigned short j=0;j<SRVO_NUM_SRVOS;j++)
    srvo[j]->gotoPos(id);
}

void cmdPark(void)
{
  for (unsigned short i=0;i<SRVO_NUM_SRVOS;i++)
    srvo[i]->goHome(HALF_SPEED);
}

void cmdHor(void)
{
  srvo[SRVO_SHLDR]->goHor(HALF_SPEED);
  srvo[SRVO_ELBW]->goHor(HALF_SPEED);
  srvo[SRVO_WRST]->goHor(5);
  /*
  for (unsigned short i=0;i<SRVO_NUM_SRVOS;i++)
    srvo[i]->goHor(HALF_SPEED);
  */
}
void cmdLow2Hor(void)
{
#if 0
  srvo[SRVO_SHLDR]->goHor(HALF_SPEED);
  srvo[SRVO_ELBW]->goHor(HALF_SPEED);
  srvo[SRVO_WRST]->goHor(5);
#else
  arm->halfLow2H();
#endif
  /*
  for (unsigned short i=0;i<SRVO_NUM_SRVOS;i++)
    srvo[i]->goHor(HALF_SPEED);
  */
}
void cmdUp2Hor(void)
{
#if 0
  srvo[SRVO_SHLDR]->goHor(HALF_SPEED);
  srvo[SRVO_ELBW]->goHor(HALF_SPEED);
  srvo[SRVO_WRST]->goHor(5);
#else
  arm->up2H();
#endif
  /*
  for (unsigned short i=0;i<SRVO_NUM_SRVOS;i++)
    srvo[i]->goHor(HALF_SPEED);
  */
}
void cmdUp(void)
{
#if 0
  srvo[SRVO_SHLDR]->goUpper(2);
  srvo[SRVO_ELBW]->goUpper(5);
  srvo[SRVO_WRST]->goUpper(6);
#else
  // load a sequence for the servos
  arm->h2Up();
#endif
  /*
  for (unsigned short i=0;i<SRVO_NUM_SRVOS;i++)
    srvo[i]->goUpper(HALF_SPEED);
  */
}
void cmdDown(void)
{
#if 0
  srvo[SRVO_SHLDR]->goLower(2);
  srvo[SRVO_ELBW]->goLower(2);
  srvo[SRVO_WRST]->goLower(2);
#else
  arm->h2HalfLow();
#endif
}

void cmdCCW(void)
{
  srvo[SRVO_BASE]->goCCW(HALF_SPEED);
}

void cmdCW(void)
{
  srvo[SRVO_BASE]->goCW(HALF_SPEED);
}

using namespace std;
extern int dontDie;

Servo::Servo(unsigned short home, unsigned short low, unsigned short hor, unsigned short up,  unsigned short min,unsigned short max,unsigned short myId) // constructor
{
  // constructor
  // ack, need to preload the positions for each serov.  Do it based on ID
  // ugly, but life.  And I want to just create a ptr to the various arrays
  // and the compiler is fighting me - hence the 4 assignments.  FIXME
  unsigned short i;

#if 0
  //  printf("ENUM: %d, %d, %d, %d\n",SRVO_BASE,SRVO_SHLDR,SRVO_ELBW,SRVO_WRST);
  for (i=0;i<NUM_ARM_POS;i++)
    {
      switch(myId)
	{
	case SRVO_BASE:
	  posMS[i]=base[i];
	  posSpd[i]=bSpd[i];
	  break;
	case SRVO_SHLDR:
	  posMS[i]=shoulder[i];
	  posSpd[i]=sSpd[i];
	  break;
	case SRVO_ELBW:
	  posMS[i]=elbow[i];
	  posSpd[i]=eSpd[i];
	  break;
	case SRVO_WRST:
	  posMS[i]=wrist[i];;
	  posSpd[i]=wSpd[i];
	  break;
	default:
	  printf("ERROR in MYID %d\n",myId);
	}
      
    }
#endif

  lowPos=low;
  horPos=hor;
  upPos=up;
  minPos=min;
  maxPos=max;
  estop=false;
  homePos=home;
  trgPos=curPos=home;
  id=myId; // save this
  forceHome(); // we dont know starting position - so lets force it
}

void Servo::updateMidPos(void)
{
#if 0
  if (trgPos>curPos)
    midPos=curPos+SRVO_INC;
  else if (trgPos<curPos)
    midPos=curPos-SRVO_INC;
#else
  if (trgPos>curPos)
    midPos=curPos+movInc;
  else if (trgPos<curPos)
    midPos=curPos-movInc;
#endif
}  

unsigned short Servo::validPos(unsigned short pos)
{
  unsigned short valid=false; //default to bad
  if (pos>=minPos  && pos<=maxPos)
    valid=true;
  return valid;
}

unsigned short Servo::gotoMS(unsigned short pos,unsigned short spd,unsigned short microStep)
 {
  // this doesnt move instantaneously.  It sets up a state engine to move
  // at a specific speed.  Over time, we get there.
  // also, we have to make sure that we dont go beyond the current limit

  // but its trickier than that.  If we are scanning, we move more slowly.
  // The only way to move more slowly is to move in microsteps

  char uBuf[20];

  if (eStop)
    return -1;


  if (validPos(pos))
  {
  trgPos=pos;
  speed=spd;
  uStep=microStep;
  // speed is the number of milliseconds in between updates

  // SRVO_INC is the number of usecs we move at each update
  // which direction?
#if 0
  if (trgPos>curPos)
    movInc=(ROT_CONST*speed);
  else if (trgPos<curPos)
    movInc=0-(ROT_CONST*speed);
#else
  if (trgPos!=curPos)
    movInc=ROT_CONST*speed;
#endif

  if (uStep==true)
	{
	  // have to move just a bit.  Move at 5 us intervals, and wait a sec.
	  updateMidPos();
	  if (midPos>=ABS_SERVO_MIN && midPos<=ABS_SERVO_MAX)
	    sprintf(uBuf,"#%dP%d\r",id,midPos);
	}
      else
	{
	  // this just goes to the final position w/o micro steps
	  if (trgPos>=ABS_SERVO_MIN && trgPos<=ABS_SERVO_MAX)
	    sprintf(uBuf,"#%dP%d\r",id,trgPos);
	}
      txUartNull((unsigned char *) uBuf);
    }
  else
    {
      // illegal destination
    }
}

void Servo::gotoPos(unsigned short position)
{
  if (position>=0 && position<=NUM_ARM_POS)
    gotoMS(getPosMS(id,position),getPosSpd(id,position),true);
}


void Servo::arrived(void)
{
  // we have arrived at a target position
  // if we are in a sequence, load the next seq pos
}

void Servo::moveMe(void)
{
  // callback to move the servo
  // this is a timertick based function called every XXms
  unsigned short i;
  char uBuf[20];
  if (eStop)
    return;
  if (trgPos!=curPos)
  {
    // we are not yet there.
    //    if (speed==FULL_SPEED)
    if (uStep==false)
      {
	// just one command to go there
	if (abs(movInc)>=(abs(curPos-trgPos)))
	  curPos=trgPos; // close enough
	else
	  curPos+=movInc;
      }
    else
      {
	// not moving at full speed
	// time to send another move command
	curPos=midPos; // we moved a bit

	if (abs(movInc)>=(abs(curPos-trgPos)))
	  {
	    // we have arrived
	    curPos=midPos=trgPos; // close enough
	    // now can call back the servo object
	    // he might want to move to another position
	  }
	else if (midPos>=ABS_SERVO_MIN && midPos<=ABS_SERVO_MAX)
	  {
	    // here we find the next increment point, based on the speed
	    // update the midpos to be the next micro-step location
	    
	    updateMidPos();
	    sprintf(uBuf,"#%dP%d\r",id,midPos);
	    txUartNull((unsigned char *) uBuf);
	  }
      }
  }
}


unsigned short Servo::getCnt(void)
{
  return cntDown;
}
unsigned short Servo::getPos(void)
{
  return curPos;
}
unsigned short Servo::getTrg(void)
{
  return trgPos;
}
unsigned short Servo::getMin(void)
{
  return minPos;
}
unsigned short Servo::getMax(void)
{
  return maxPos;
}


void Servo::forceHome(void)
{
  gotoMS(homePos,FULL_SPEED,false);
}

void Servo::goHome(unsigned short speed)
{
  gotoMS(homePos,speed,true);
}
void Servo::goUpper(unsigned short speed)
{
  gotoMS(upPos,speed,true);
}
void Servo::goLower(unsigned short speed)
{
  gotoMS(lowPos,speed,true);
}
void Servo::goHor(unsigned short speed)
{
  gotoMS(horPos,speed,true);
}
void Servo::goCCW(unsigned short speed)
{
  gotoMS(getPos()+ SERVO_CLI_INC,speed,true);
}
void Servo::goCW(unsigned short speed)
{
  gotoMS(getPos()- SERVO_CLI_INC,speed,true);
}


void Servo::stop(unsigned short goNogo)
{
  // freeze/start all motion.
  estop=goNogo;
}


int Arm::seqLoad(short seq)
{
  int ret=0;
  unsigned short pos, spd,i;

  if (seqPos==NULL || seqSpd==NULL)
    return -1;

  pos=seqPos[seq][0];
  if (pos==0)
    {
      // the sequence is over.
      // do nothing and return 1
      //printf("done with seq\n");
      ret=1;
    }
  else {
    spd=seqSpd[seq][0];
    srvo[SRVO_SHLDR]->gotoMS(pos,spd,true);

    pos=seqPos[seq][1];
    spd=seqSpd[seq][1];
    srvo[SRVO_ELBW]->gotoMS(pos,spd,true);

    pos=seqPos[seq][2];
    spd=seqSpd[seq][2];
    srvo[SRVO_WRST]->gotoMS(pos,spd,true);
  }
  return ret;
}

Arm::Arm(void)
{
  // arm constructor
  seqNum=-1; // not in a sequence
  armPos=POS_LOW; // presume we are low
  seqPos=NULL;
  seqSpd=NULL;
  cntr=0;
}
#define ARM_SEQ_CNT 40//2 second
void Arm::update(void)
{
  // this gets called periodically. - every 50 ms
  if (eStop)
    return;

  if (cntr>0)
    {
      // in a sequence
      if (--cntr==0)
	{
	  // and the counter triggered
	  seqNum++;
	  if (seqLoad(seqNum))
	    {
	      // we are done with the sequence
	      seqNum= -1;
	      // can leave the cntr at 0
	    }
	  else
	    cntr=ARM_SEQ_CNT; // and reload the counter
	}
    }
}
void Arm::h2Up( void)
{
  //set up the sequence to move the arm up
  //  printf("in Seq\n");
  if (seqNum== -1)
    {
      seqPos=seqPosH2U;
      seqSpd=seqSpdH2U;
      // not in a seq now
      cntr=ARM_SEQ_CNT; // wait this long
      seqNum=0;
      seqLoad(seqNum);
    }
}
void Arm::up2H(void)
{
  //set up the sequence to move the arm up
  //  printf("in Seq\n");
  if (seqNum== -1)
    {
      // not in a seq now
      seqPos=seqPosU2H;
      seqSpd=seqSpdU2H;

      cntr=ARM_SEQ_CNT; // wait this long
      //      seqNum=9;
      seqLoad(seqNum);
    }
}
void Arm::h2HalfUp( void)
{
  //set up the sequence to move the arm up
  //  printf("in Seq\n");
  if (seqNum== -1)
    {
      seqPos=seqPosH2HalfU;
      seqSpd=seqSpdH2HalfU;
      // not in a seq now
      cntr=ARM_SEQ_CNT; // wait this long
      seqNum=0;
      seqLoad(seqNum);
    }
}
void Arm::halfUp2H(void)
{
  //set up the sequence to move the arm up
  //  printf("in Seq\n");
  if (seqNum== -1)
    {
      // not in a seq now
      seqPos=seqPosHalfU2H;
      seqSpd=seqSpdHalfU2H;

      cntr=ARM_SEQ_CNT; // wait this long
      //      seqNum=9;
      seqLoad(seqNum);
    }
}
void Arm::h2Low(void)
{
  //set up the sequence to move the arm up
  //  printf("in Seq\n");
  if (seqNum== -1)
    {
      // not in a seq now
      seqPos=seqPosH2L;
      seqSpd=seqSpdH2L;

      cntr=ARM_SEQ_CNT; // wait this long
      //      seqNum=18;
      seqLoad(seqNum);
    }
}
void Arm::low2H(void)
{
  //set up the sequence to move the arm up
  //  printf("in Seq\n");
  if (seqNum== -1)
    {
      // not in a seq now
      seqPos=seqPosL2H;
      seqSpd=seqSpdL2H;

      cntr=ARM_SEQ_CNT; // wait this long
      //      seqNum=27;
      seqLoad(seqNum);
    }
}
void Arm::h2HalfLow(void)
{
  //set up the sequence to move the arm up
  //  printf("in Seq\n");
  if (seqNum== -1)
    {
      // not in a seq now
      seqPos=seqPosH2HalfL;
      seqSpd=seqSpdH2HalfL;

      cntr=ARM_SEQ_CNT; // wait this long
      //      seqNum=18;
      seqLoad(seqNum);
    }
}
void Arm::halfLow2H(void)
{
  //set up the sequence to move the arm up
  //  printf("in Seq\n");
  if (seqNum== -1)
    {
      // not in a seq now
      seqPos=seqPosHalfL2H;
      seqSpd=seqSpdHalfL2H;

      cntr=ARM_SEQ_CNT; // wait this long
      //      seqNum=27;
      seqLoad(seqNum);
    }
}
