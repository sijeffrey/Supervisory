/*
  Supervisory Alarm System
  revision 1.0

  Copyright (c) 2016, Simon Jeffrey, All rights reserved

  This program is free software: you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// Program scans PORT B and if low level detected will flash corresponding pin
// on PORT A. Also an alert tone and output HIGH is send to alert the user.
// Once the ACCEPT button is pressed (goes LOW) the PORT A pin will stay HIGH.
// Also the external alert tone and output pin will mute and go LOW. Upon the 
// RESET button being pressed (goes LOW) and if the input is healthy (HIGH) 
// then the PORT A pin will extinguish and become available for future alarms.
// There is also a lamp test facility which drives all PORT B pins HIGH while
// pressed (goes LOW). Serial data is output as JSON format for SCADA update.

#include "Wire.h"

// Local IO

const int acceptPin = 2;
const int resetPin = 3;
const int lampTestPin = 4;
const int buzzerPin = 5;
const int healthPin = 13;
const int speakerPin = 8;
const int freq = 2000;

// REMOTE IO

const int chipCount = 2;
const int totalRegisters = 3;

// VARAIBLES

int cycles = 0;
byte inputs = 0;
byte FLASHER = 0;

int chip[chipCount] = { 0x20, 0x21 }; // addresses of MCP23017 GPIO chips

// REGISTERS

// flashing lamp & alarm active

byte flashReg[totalRegisters] = { 0x00, 0x00, 0x00 }; // initialise flash groups 
byte alarmReg[totalRegisters] = { 0x00, 0x00, 0x00 }; // initialise alarm groups

// steady lamp & alarm acknowledged

byte steadyReg[totalRegisters] = { 0x00, 0x00, 0x00 }; // initialise steady group

// set up pins

void setup() {

  // set up local pins
  pinMode(acceptPin, INPUT);
  pinMode(resetPin, INPUT);
  pinMode(lampTestPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(healthPin, OUTPUT);
  
  // set up serial
  Serial.begin(115200);
  
  // pins 15~17 to GND, I2C bus address is 0x20
  Wire.begin(); // wake up I2C bus

  // loop over the chip arrays and set them up:
  for (int ic = 0; ic < chipCount; ic++) {
  Wire.beginTransmission(chip[ic]);
    Wire.write(0x00); // IODIRA register
    Wire.write(0x00); // set all of bank A to outputs
    Wire.endTransmission();
  }
}

void flash() 
{
  // flasher (ON & OFF)
  cycles++;
  if (cycles == 5) {
    cycles = 0;
    digitalWrite(healthPin, !digitalRead(healthPin));
    if (FLASHER == 0xFF) {
      FLASHER = 0x00;
    } 
    else {
      FLASHER = 0xFF;
    }
  }
  // add some delay for flasher
  delay(100); // 100ms delay (gives about half second flash)
}

void alarms(int ic) 
{
  // check inputs to activation & flash & buzzer ON
  // loop over the input array and check for low:
  // read the inputs of bank B
  Wire.beginTransmission(chip[ic]);
  Wire.write(0x13);
  Wire.endTransmission();
  Wire.requestFrom(chip[ic], 1);
  inputs=Wire.read();
  for (int i=0; i < 8; i++) {
    if ((inputs & (1 << i)) == 0) {
      if ((steadyReg[ic] & (1 << i)) == 0) {
        flashReg[ic] |= 1 << i;
        digitalWrite(buzzerPin, HIGH); 
        tone(speakerPin, freq); 
        if ((alarmReg[ic] & (1 << i)) == 0) {
          Serial.print("{\"event\":\"alarm\",\"chip\":");
          Serial.print(ic, DEC);
          Serial.print(",\"pin\":");          
          Serial.print(i, DEC);
          Serial.print("}\n");
          alarmReg[ic] |= 1 << i;
        }
      }
    }
  }
  
  // check for accept button pressed and set steady lamp & buzzer OFF
  // loop over the flash array and check for accept button pressed:
  for (int i=0; i < 8; i++) {
    if ( (flashReg[ic] & (1 << i)) && !digitalRead(acceptPin) ) {
      flashReg[ic] &= ~(1 << i);
      steadyReg[ic] |= 1 << i;
      digitalWrite(buzzerPin, LOW);
      noTone(speakerPin);
      Serial.print("{\"event\":\"accept\",\"chip\":");
      Serial.print(ic, DEC);
      Serial.print(",\"pin\":");          
      Serial.print(i, DEC);
      Serial.print("}\n");
    }
  }

  // check for reset button pressed and clear all related registers
  // loop over the steady array and check for reset button pressed:
  for (int i=0; i < 8; i++) {
    if ( (steadyReg[ic] & (1 << i)) && !digitalRead(resetPin) && (inputs & (1 << i)) ) {
      steadyReg[ic] &= ~(1 << i);
      alarmReg[ic] &= ~(1 << i);
      Serial.print("{\"event\":\"reset\",\"chip\":");
      Serial.print(ic, DEC);
      Serial.print(",\"pin\":");          
      Serial.print(i, DEC);
      Serial.print("}\n");
    }
  }

  // light related lamps
  // loop over the lamp array and set them all to flash array or steady array values:
  Wire.beginTransmission(chip[ic]);
  Wire.write(0x12); // GPIOA
  if (!digitalRead(lampTestPin)) {
    Wire.write(0xff);   
  }
  else {
    Wire.write(((flashReg[ic] & FLASHER) | steadyReg[ic]));    // bank A set to flashReg & FLASHER (OR) steadyReg
  }
  Wire.endTransmission();
}

// run main program

void loop()
{
  // call alarm main function
  for (int ic = 0; ic < chipCount; ic++) {
    alarms(ic);
  }
  // set flasher variable and delay a bit
  flash(); 
}

