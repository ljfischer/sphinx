#ifndef Servo_h
#define Servo_h

// arm geometry
#define UPARM_LEN   16
#define FOREARM_LEN 12
#define ARM_Y_LEN   16
#define ARM_X_LEN   12
#define HOLDER_LEN  4


// speeds
#define QTR_SPEED 1 // ms increments
#define HALF_SPEED 2 // ms increments
#define THREEQTR_SPEED 3 // ms increments
#define FULL_SPEED 4 // ms increments

#define ROT_CONST 1.5 // used to calculate the current position at full speed
//#define ROT_CONST 1 // used to calculate the current position at full speed

#define SRVO_INC 5
#define SRVO_TIMER_TICK 50 // ms between movement callback
#define SERVO_CLI_INC 50 // the amount we move per cli +/-

#define HOR_DELAY 850
#define VER_DELAY 10000


#define ROT_0_MS     990
#define ROT_135_MS   1500
#define ROT_225_MS   1810
#define ROT_450_MS   2475
#define ROT_360_MS   2180



#define HOME_PLAT_ANGLE 270
#define MAX_ANGLE 360
#define MAX_SERVO_MS 2500
#define MIN_SERVO_MS 500
#define ABS_SERVO_MIN 600
#define ABS_SERVO_MAX 2400

#define ROT_INC 2

#define SCULPT_HOR 500 // 600 ms=about 180 degree

#define SRVO_BASE_MIN  600
#define SRVO_BASE_MAX  2500
#define SRVO_BASE_HOME  710
//#define SRVO_BASE_135  1180
//#define SRVO_BASE_225  1600
#define SRVO_BASE_135  1190
#define SRVO_BASE_225  1480
#define SRVO_BASE_450  2300
#define SRVO_BASE_360  1900

#if 0 // table mount

#define SRVO_SHLDR_MIN  800
#define SRVO_SHLDR_MAX  1550
#define SRVO_SHLDR_HOME  1000
#define SRVO_SHLDR_LOWER  1220
#define SRVO_SHLDR_HOR  1030
#define SRVO_SHLDR_UPPER  1180

#define SRVO_ELBW_MIN  900
#define SRVO_ELBW_MAX  1400
#define SRVO_ELBW_HOME  1000
#define SRVO_ELBW_LOWER  1310
#define SRVO_ELBW_HOR  1230
#define SRVO_ELBW_UPPER  1030

#define SRVO_WRST_MIN  900
#define SRVO_WRST_MAX  1750
#define SRVO_WRST_HOME  1120
#define SRVO_WRST_LOWER  1670
#define SRVO_WRST_HOR  1680
#define SRVO_WRST_UPPER  1180
#else
// Inverted mount
#define SRVO_SHLDR_MIN  900
#define SRVO_SHLDR_MAX  1550
//#define SRVO_SHLDR_LOWER  1120 // 1
//#define SRVO_SHLDR_LOWER  1170 //2
//#define SRVO_SHLDR_LOWER  1220 //3
#define SRVO_SHLDR_LOWER  1270 //4
//#define SRVO_SHLDR_HOR  930 //1
//#define SRVO_SHLDR_HOR  980 //2
//#define SRVO_SHLDR_HOR  1030 //3
#define SRVO_SHLDR_HOR  1050 //4
//#define SRVO_SHLDR_UPPER  1130 //1
//#define SRVO_SHLDR_UPPER  1130 //2
#define SRVO_SHLDR_UPPER  1050 //4

#define SRVO_SHLDR_HOME  SRVO_SHLDR_LOWER

#define SRVO_SHLDR_135H  SRVO_SHLDR_HOR
#define SRVO_SHLDR_135V  SRVO_SHLDR_UPPER
#define SRVO_SHLDR_225H SRVO_SHLDR_HOR
#define SRVO_SHLDR_225V SRVO_SHLDR_UPPER
#define SRVO_SHLDR_450H  SRVO_SHLDR_HOR
#define SRVO_SHLDR_360H  SRVO_SHLDR_HOR

#define SRVO_ELBW_MIN  950
#define SRVO_ELBW_MAX  1900
//#define SRVO_ELBW_LOWER  1210 // 1
//#define SRVO_ELBW_LOWER  1290 //2
//#define SRVO_ELBW_LOWER  1360 //3
#define SRVO_ELBW_LOWER  1760 //3
//#define SRVO_ELBW_HOR  1230 //1
//#define SRVO_ELBW_HOR  1180 //2
//#define SRVO_ELBW_HOR  1180 //3
#define SRVO_ELBW_HOR  1550 //4
//#define SRVO_ELBW_UPPER  950  //1
//#define SRVO_ELBW_UPPER  950 //2

#define SRVO_ELBW_HOME  SRVO_ELBW_LOWER

#define SRVO_ELBW_UPPER  1170 //4
#define SRVO_ELBW_135H  SRVO_ELBW_HOR
#define SRVO_ELBW_135V  SRVO_ELBW_UPPER
#define SRVO_ELBW_225H SRVO_ELBW_HOR
#define SRVO_ELBW_225V SRVO_ELBW_UPPER
#define SRVO_ELBW_450H  SRVO_ELBW_HOR
#define SRVO_ELBW_360H  SRVO_ELBW_HOR

#define SRVO_WRST_MIN  900
#define SRVO_WRST_MAX  1900
//#define SRVO_WRST_LOWER  1670 //1
//#define SRVO_WRST_LOWER  1670 //2
//#define SRVO_WRST_LOWER  1670 //3
#define SRVO_WRST_LOWER  1790 //4
//#define SRVO_WRST_HOR  1680 //1
//#define SRVO_WRST_HOR  1600 //2
//#define SRVO_WRST_HOR  1580 //3
#define SRVO_WRST_HOR  1690 //4
//#define SRVO_WRST_UPPER  1180 //1
//#define SRVO_WRST_UPPER  1130 //2
//#define SRVO_WRST_UPPER  1180 //3
#define SRVO_WRST_UPPER  1200 //4

#define SRVO_WRST_HOME  SRVO_WRST_LOWER

#define SRVO_WRST_135H  SRVO_WRST_HOR
#define SRVO_WRST_135V  SRVO_WRST_UPPER
#define SRVO_WRST_225H SRVO_WRST_HOR
#define SRVO_WRST_225V SRVO_WRST_UPPER
#define SRVO_WRST_450H SRVO_WRST_HOR
#define SRVO_WRST_360H SRVO_WRST_HOR
#endif


// the different positions for the arm
enum {
  ARM_POS_0,
  ARM_POS_135H1,
  ARM_POS_135V,
  ARM_POS_135H2,
  ARM_POS_225H1,
  ARM_POS_225V,
  ARM_POS_225H2,
  ARM_POS_450,
  ARM_POS_360,
  NUM_ARM_POS
};
  
enum {
  SRVO_BASE,
  SRVO_SHLDR,
  SRVO_ELBW,
  SRVO_WRST,
  SRVO_NUM_SRVOS
};

enum {
  POS_LOW,
  POS_HOR,
  POS_UP,
  POS_NUM_POS
};

typedef unsigned short ARRAY_OF_3_USHORT[3];

class Arm{
 public:
  Arm(void);
  void update(void);
  void h2Up(void);
  void up2H(void);
  void h2HalfUp(void);
  void halfUp2H(void);
  void h2Low(void);
  void low2H(void);
  void h2HalfLow(void);
  void halfLow2H(void);
  int seqNum; //if we are engaged in a sequence
  unsigned short cntr; // 
 private:
  int seqLoad(short seq);
  ARRAY_OF_3_USHORT *seqPos;
  ARRAY_OF_3_USHORT *seqSpd;

  unsigned short armPos; //low, hor, up
};


class Servo {
 public:
  Servo(unsigned short home, unsigned short low, unsigned short hor, unsigned short up, unsigned short min,unsigned short max,unsigned short myId); // constructor
  void gotoPos(unsigned short pos);

  unsigned short gotoMS(unsigned short pos,unsigned short spd,unsigned short microStep);

  void goHome(unsigned short speed);
  void goLower(unsigned short speed);
  void goHor(unsigned short speed);
  void goUpper(unsigned short speed);
  void goCCW(unsigned short speed);
  void goCW(unsigned short speed);
  void stop(unsigned short goNogo);
  unsigned short getPos(void); // the current position
  unsigned short getTrg(void); // the target position
  unsigned short getMin(void);
  unsigned short getMax(void);
  unsigned short getCnt(void);
  void arrived(void); // we arrived at a location 
  void moveMe(void);
  void updateMidPos(void);
  void forceHome(void);
  unsigned short validPos(unsigned short pos);
  //  unsigned short pos(unsigned short id);
  //  unsigned short posSpd[NUM_ARM_POS]; // the speed to get to the various positions
 private:
  //  unsigned short posMS[NUM_ARM_POS]; // the servo setting for the various positions
  int commFS; // communications link to talk to the servo
  unsigned short uStep; // are we microstepping?  At this point, every move does
  unsigned short id; // my id
  unsigned short estop; // emergency stop
  unsigned short homePos, curPos,trgPos,midPos,lowPos,upPos,horPos;
  unsigned short minPos,maxPos;
  unsigned short speed; // the speed at which the servo is being moved
  unsigned short cntDown; // the current countdown til it reaches its destination
  unsigned short movInc; // the ms to increment each micro step
};

extern unsigned short base[NUM_ARM_POS];
extern unsigned short shoulder[NUM_ARM_POS];
extern unsigned short elbow[NUM_ARM_POS];
extern unsigned short wrist[NUM_ARM_POS];

extern unsigned short bSpd[NUM_ARM_POS];
extern unsigned short sSpd[NUM_ARM_POS];
extern unsigned short eSpd[NUM_ARM_POS];
extern unsigned short wSpd[NUM_ARM_POS];

extern unsigned short calTime[NUM_ARM_POS];

extern short eStop;

void calibrate(void);
void sculpt(void);
void cmdCCW(void);
void cmdCW(void);
void cmdPark(void);
void cmdHor(void);
void cmdUp2Hor(void);
void cmdLow2Hor(void);
void cmdUp(void);
void cmdDown(void);
void gotoPos(unsigned short id);


#endif // Servo_h
