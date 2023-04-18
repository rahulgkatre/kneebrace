#include <Wire.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;
uint8_t effect;
unsigned long last_reading_hap = 0;

void hapOff() {
  drv.setWaveform(0,0);
  drv.go();
  return;
}
void hapMedium() {
  drv.setWaveform(0,119);
  drv.go();
  return;
}
void hapHigh() {
  drv.setWaveform(0,118);
  drv.go();
  return;
}

void hapticSetup() {
//  Serial.begin(9600);
//  Serial.println("Adafruit DRV2605 Basic test");
  if (! drv.begin()) {
    Serial.println("Could not find DRV2605");
//    while (1) delay(10);
  }
 
  drv.selectLibrary(1);
  
  // I2C trigger by sending 'go' command 
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG); 
  return;
}

void hapticLoop() {
  int curr = millis();
  if (curr - last_reading_hap >= HAPTIC_RATE_MS - UPDATE_RATE_CORRECTION) {
    drv.go();
    last_reading_hap = curr;
  }
////  Serial.print("Effect #"); Serial.println(effect);
//
//  // set the effect to play
//    // play effect 
//  drv.setWaveform(1, 0);       // end waveform
//
//  // play the effect!
//  drv.go();
//
//  // wait a bit
//  delay(500);
  return;
}
