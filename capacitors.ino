#include "Adafruit_MPR121.h"

Adafruit_MPR121 capacitor = Adafruit_MPR121();

//Active capacitors
uint16_t lastactive = 0;
uint16_t curractive = 0;

void init(){
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D

  if (!capacitor.begin(0x5A)) {
    Serial.println("MPR121 connection failed");
  }
}

uint16_t capacitor_read() {
  lastactive = curractive;
  curractive = capacitor.touched();
  return curractive;
}

void capacitor_print_status() {
  Serial.printf("Current Active: %d  \t Last Active %d \n", curractive, lastactive);
}

bool any_touched() {
  return curractive != 0;
}

bool capacitor_touched(int pad) {
  return (curractive & _BV(pad));
}

bool capacitor_turned_on(int pad) {
  return (capacitor_touched(pad) && !(lastactive & _BV(pad)));
}

bool capacitor_turned_off(int pad) {
  return (!capacitor_touched(pad) && (lastactive & _BV(pad)));
}
