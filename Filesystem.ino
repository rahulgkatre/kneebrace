#define FORMAT_LITTLEFS_IF_FAILED false

String readFile(fs::FS &fs, const char* path) {
    Serial.printf("Reading file: %s\r\n", path);
    String output;

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
    }

    Serial.println("- read from file:");
    while(file.available()){
        //Serial.write(file.read());
        char intRead = file.read();
        output += intRead;
    }

    file.close();
    return output;    
}

void writeFile(fs::FS &fs, const char* path, const char* message) {
    Serial.printf("Writing file: %s\r\n", path);
    File file = fs.open(path, FILE_WRITE); // <<<<< HERE
    // File file = fs.open(path, FILE_APPEND); // << TO THIS
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

void appendToFile(File file, const char* message) {
    if(file.print(message)){
        Serial.println("- file appended");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

bool fileSystemSetup() {
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        Serial.println("LittleFS Mount Failed");
        return false;
    }

    return true;
}
