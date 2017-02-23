#ifndef Ranger_h
#define Ranger_h

void initGPIO(void);
void * ranger(void* threadid);


// ranger
#define MAX_RANGE 200
#define MIN_RANGE 3
#define HOR_SCAN_DIST 60  // horizontal scan distance.  60CM - About 2'
#define VER_SCAN_DIST 30  // vertical scan distance. 30cm - about 1'

/*
 *Note that wiringPi maps pins in its own weird way.
 * pins 4, and 5 below, are gpio 23 and 24, and phys 16, 18
 *  - for this implementation, pin 24 goes thru a voltage divider of 2k/1k from echo
 */

#define TRIG_PIN 4
#define ECHO_PIN 5

#endif // Ranger_h
