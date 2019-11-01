#include <QueueArray.h>

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "var.h"

#include "MPU6050_6Axis_MotionApps20.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 mpu;
//MPU6050 mpu_2(0x69); // <-- use for AD0 high

#define INTERRUPT_PIN 7

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };

const int INDEX_FINGER = 2;
const int MIDDLE_FINGER = 3;
const int RING_FINGER = 4;
const int PINKY_FINGER = 5;

const int SIDE_FINGER = 6;

const int WOBBLE_THRESHOLD = 35;
const int WOBBLE_COUNT = 50;

const int VOLUME_THRESHOLD = 5000;
const int VOLUME_COUNT = 30;

const int VOLUME_DECREASE_THRESHOLD = -2000;
const int VOLUME_DECREASE_COUNT = 10;

const double GRAVITY_FACTOR = 9.2; // SHOULD BE 9.8
const double HORIZONTAL_FACTOR = 4.5; // SHOULD BE 0
const double OUT_FACTOR = 2; // SHOULD BE 0;

const double ACCEL_Z_RESET_COUNT = HISTORY_LENGTH - 5;

float TEMPO = 700;
float noteStartTime;

bool isNotePlaying = false;

int channel = 1; // Defines the MIDI channel to send messages on (values from 1-16)
int velocity = 50; // Defines the velocity that the note plays at (values from 0-127)

int32_t current_note [2];
int current_note_spot = 0;

struct Acceleration accel;

void setup() {
  mpu_init(mpu, INTERRUPT_PIN);
  cap_init();

  Serial.println("Waiting...");
  // first data points are incorrect and need to clear buffer
  while (millis() < 6500) {
    mpu_setup();
    getLinearAccel(&accel);
  }
  Serial.println("Done");
}

int accelSpot = 0;
void printArray (double array []) {
  Serial.println("Print Array");
  for (int i=0;i < HISTORY_LENGTH;i++) {
    Serial.printf(" %f",array[i]);
  }
  Serial.println("\n");
}

int FilledSpot = 0;

double AX_HIST[HISTORY_LENGTH];
double AY_HIST[HISTORY_LENGTH];
double AZ_HIST[HISTORY_LENGTH];

void addAccel (double ax, double ay, double az) {
  AX_HIST[FilledSpot] = ax;
  AY_HIST[FilledSpot] = ay;
  AZ_HIST[FilledSpot] = az;

  FilledSpot++;
  if (FilledSpot == HISTORY_LENGTH) {
    FilledSpot = 0;
  }
}

void clearArray (double * data) {
  for (int i = 0; i < HISTORY_LENGTH; i++) {
    data[i] = 0;
  }
}

bool DetectThreshold (double * data, double magnitude, int count_requirement, bool comparator = true) {
  int count = 0;
  //int len = sizeof(data)/sizeof(data[0]); //takes length of pointer instead of array
  int len = HISTORY_LENGTH;
  for (int i = 0; i < len; i++ ) {
    if (data[i] > magnitude && comparator) {
      count++;
    }
    else if (data[i] < magnitude && !comparator) {
      count++;
    }

    if (count > (count_requirement-1)) {
      return true;
    }
  }
  return false;
}

bool any_cap_turned_on(int *finger) {
  if (cap_turned_on(INDEX_FINGER)) {
    *finger = INDEX_FINGER;
    return true;
  }
  if (cap_turned_on(MIDDLE_FINGER)) {
    *finger = MIDDLE_FINGER;
    return true;
  }
  if (cap_turned_on(RING_FINGER)) {
    *finger = RING_FINGER;
    return true;
  }
  if (cap_turned_on(PINKY_FINGER)) {
    *finger = PINKY_FINGER;
    return true;
  }

  return false;
}

bool any_cap_turned_off(){
  return cap_turned_off(MIDDLE_FINGER) || cap_turned_off(INDEX_FINGER) || cap_turned_off(RING_FINGER) || cap_turned_off(PINKY_FINGER);
}

bool tryFindAction = false;
int active_finger;

void loop() {
  cap_read();

  if(!mpu_setup()) return;

  getLinearAccel(&accel);

  addAccel(accel.x, accel.y, accel.z);

  if (cap_turned_on(SIDE_FINGER)) clearTouchEffect();

  clearTimerEffects();


  if (any_cap_turned_on(&active_finger)) {
      tryFindAction = true;
      Serial.printf("Reseting Position %d\n\n", active_finger);
      clearArray(AZ_HIST);
      clearArray(AX_HIST);
  }
  if (any_cap_turned_off()) {
      tryFindAction = false;
  }

  if (!tryFindAction) return;

  if (DetectThreshold(AZ_HIST, AZ_THRESHOLD_UP, AZ_COUNT, true)) {
    Serial.println("Moved Up!");
    tryFindAction = false;
    printAccel(accel);
    turnOnEffect(active_finger, 0);
  } else if (DetectThreshold(AZ_HIST, AZ_THRESHOLD_DOWN, AZ_COUNT, false)) {
    Serial.println("Moved DOWN!");
    tryFindAction = false;
    printAccel(accel);
    turnOnEffect(active_finger, 2);
  } else if (DetectThreshold(AX_HIST, AX_THRESHOLD_RIGHT, AX_COUNT, true)) {
    Serial.println("Moved left!");
    tryFindAction = false;
    printAccel(accel);
    turnOnEffect(active_finger, 1);
  } else if (DetectThreshold(AX_HIST, AX_THRESHOLD_LEFT, AX_COUNT, false)) {
    Serial.println("Moved right!");
    tryFindAction = false;
    printAccel(accel);
    turnOnEffect(active_finger, 3);
  }
}
