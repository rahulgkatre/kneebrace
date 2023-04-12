// Note:
// Arduino compiles the files in this project as follows
// Concatenate KneeBrace.ino + BNO08X.ino + ... + Webserver.ino
// So after KneeBrace.ino (the .ino with same name as directory), it is in alphabetical order
// Then compile the concatenated .ino normally
// Had to put all the includes and defines in this file to avoid errors
// Setup and loop can be put in here because it just calls functions defined in other files
// If a function requires access to variables from other files, it must be further down in alphabetical ordering
// For example, functions in Webserver.ino can access variables of BNO08X.ino and Webpage.ino, but not the other way around

// Inludes for BNO08X.ino
#include <Adafruit_BNO08x.h>
#include <cmath>

// Includes for Filesystem.ino
#include <FS.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>

// Includes for FlexSensor.ino
#include <Adafruit_PCF8591.h>

// Includes for GaitAnalysis.ino

// Includes for Network.ino
#include <ESPmDNS.h>
#include <WiFi.h>

// Includes for Webserver.ino
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void setup() {
    Serial.begin(115200);
    bno08XSetup();
    flexSensorSetup();
    if (fileSystemSetup()) {
        networkSetup();
        webServerSetup();
    }
}

void loop() {
    networkLoop();
    webServerLoop();
    bno08XLoop();
    flexSensorLoop();
}
