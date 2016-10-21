// this is a ATmega program to check inputs for interuptions and then trigger output as per Bruce Supervisory System
#include "Wire.h"
// VARAIBLES
int cycles = 0;byte inputs = 0;byte FLASHER = 0;
// Remote IO
const int rowCount = 3;const int colCount = 8;const int chipCount = 3;
byte chip[chipCount] = { 0x20, 0x21, 0x22 };byte bank[2] = { 0x12, 0x13 };
int alarmPin[rowCount][colCount] = {  { 1, 2, 3, 4, 5, 6, 7, 8 }, // set row 1  { 1, 2, 3, 4, 5, 6, 7, 8 }, // set row 2  { 1, 2, 3, 4, 5, 6, 7, 8 }  // set row 3};
int lampPin[3][8] = {  { 1, 2, 3, 4, 5, 6, 7, 8 }, // set row 1  { 1, 2, 3, 4, 5, 6, 7, 8 }, // set row 2  { 1, 2, 3, 4, 5, 6, 7, 8 }  // set row 3};
// Local IO
int acceptPin = 1;int resetPin = 2;int lampTestPin = 3;int buzzerPin = 4;int healthPin = 5;
// REGISTERS
// flashing lamp & alarm active
int flashReg[3][8] = {  { 0, 0, 0, 0, 0, 0, 0, 0 }, // initialise group 1  { 0, 0, 0, 0, 0, 0, 0, 0 }, // initialise group 2  { 0, 0, 0, 0, 0, 0, 0, 0 }  // initialise group 3};
// steady lamp & alarm acknowledged
int steadyReg[3][8] = {   { 0, 0, 0, 0, 0, 0, 0, 0 },   { 0, 0, 0, 0, 0, 0, 0, 0 },   { 0, 0, 0, 0, 0, 0, 0, 0 } };
// set up pins
void setup() {
  // loop over the pin array and set them all to output:  for (int row = 0; row < rowCount; row++) {    for (int col = 0; col < colCount; col++) {        pinMode(lampPin[row][col], OUTPUT);    }  }  // loop over the pin array and set them all to input:  for (int row = 0; row < rowCount; row++) {    for (int col = 0; col < colCount; col++) {        pinMode(alarmPin[row][col], INPUT);    }  }  // set up local pins  pinMode(acceptPin, INPUT);  pinMode(resetPin, INPUT);  pinMode(lampTestPin, INPUT);  pinMode(buzzerPin, OUTPUT);  pinMode(healthPin, OUTPUT);  // set up serial  Serial.begin(9600);  // pins 15~17 to GND, I2C bus address is 0x20  Wire.begin(); // wake up I2C bus
  // loop over the chip arrays and set them up:  for (int ic = 0; ic < chipCount; ic++) {    Wire.beginTransmission(chip[ic]);    Wire.write(0x00); // IODIRA register    Wire.write(0x00); // set all of bank A to outputs    Wire.endTransmission();  }}
void alarms() {
  // flasher (ON & OFF)  cycles++;  if (cycles == 10) {    cycles = 0;    digitalWrite(healthPin, !digitalRead(healthPin));    FLASHER = !FLASHER;  }
  // check inputs to activation & flash & buzzer ON  // loop over the input array and check for low:  for (int row = 0; row < rowCount; row++) {    for (int col = 0; col < colCount; col++) {        if (!digitalRead(alarmPin[row][col])) {   // IF ALARM PIN NOT HIGH        flashReg[row][col] = true;        digitalWrite(buzzerPin, HIGH);      }    }  }
  // check for accept button pressed and set steady lamp & buzzer OFF  // loop over the flash array and check for accept button pressed:  for (int row = 0; row < rowCount; row++) {    for (int col = 0; col < colCount; col++) {        if ((flashReg[row][col]) && (digitalRead(acceptPin))) {        steadyReg[row][col] = true;        digitalWrite(buzzerPin, LOW);      }    }  }
  // check for reset button pressed and clear all related registers  // loop over the steady array and check for reset button pressed:  for (int row = 0; row < rowCount; row++) {    for (int col = 0; col < colCount; col++) {        if ((steadyReg[row][col]) && (digitalRead(resetPin)) && alarmPin[row][col]) {        steadyReg[row][col] = false;        flashReg[row][col] = false;      }    }  }
  // light related lamps  // loop over the lamp array and set them all to flash array or steady array values:  for (int row = 0; row < rowCount; row++) {    for (int col = 0; col < colCount; col++) {      digitalWrite(lampPin[row][col], (flashReg[row][col] && FLASHER) || (steadyReg[row][col]));    }  }}
// run main program
void loop(){  // do main program function  alarms();  // read the inputs of bank B  Wire.beginTransmission(0x20);  Wire.write(0x13);  Wire.endTransmission();  Wire.requestFrom(0x20, 1);  inputs=Wire.read();
}
