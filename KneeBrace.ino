// Note:
// Arduino compiles the files in this project as follows
// Concatenate KneeBrace.ino + BNO08X.ino + ... + Webserver.ino
// So after KneeBrace.ino (the .ino with same name as directory), it is in alphabetical order
// Then compile the concatenated .ino normally
// Had to put all the includes and defines in this file to avoid errors
// Setup and loop can be put in here because it just calls functions defined in other files
// If a function requires access to variables from other files, it must be further down in alphabetical ordering
// For example, functions in Webserver.ino can access variables of BNO08X.ino and Webpage.ino, but not the other way around

// Inludes and defines for BNO08X.ino
#include <Adafruit_BNO08x.h>
// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9
// For SPI mode, we also need a RESET
// #define BNO08X_RESET 5
// but not for I2C or UART
#define BNO08X_RESET -1
// #define FAST_MODE
#ifdef FAST_MODE
// Top frequency is reported to be 1000Hz (but freq is somewhat variable)
sh2_SensorId_t reportType = SH2_GYRO_INTEGRATED_RV;
long reportIntervalUs = 2000;
#else
// Top frequency is about 250Hz but this report is more accurate
sh2_SensorId_t reportType = SH2_ARVR_STABILIZED_RV;
long reportIntervalUs = 5000;
#endif

// Incluedes and defines for Filesystem.ino
#include <FS.h>
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED false

// Includes and defines for GaitAnalysis.ino
// Add this when done @Srihari

// Includes and defines for Network.ino
#include <WiFi.h>

// Includes and defines for Webpage.ino
// None at this time

// Includes and defines for Webserver.ino
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void setup()
{
    // Serial port for debugging purposes
    Serial.begin(115200);
    bno08x_setup();
    if (!filesystem_setup()) {
        return;
    }

    network_setup();
    webserver_setup();
}

void loop()
{
    webserver_loop();
}
