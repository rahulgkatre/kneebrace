#define FORMAT_LITTLEFS_IF_FAILED false

bool fileSystemSetup() {
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        Serial.println("LittleFS Mount Failed");
        return false;
    }
    return true;
}
