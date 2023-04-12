char* ap_ssid = "kneebrace";
char* ap_password = "smartorthotics!";

bool usingAP = false;

void networkSetup() {
    String configString = readFile(LittleFS, "/config.json");
    JSONVar configJson = JSON.parse(configString);
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.println(WiFi.macAddress());
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        if (configJson["networks"].hasOwnProperty(ssid)) {
            for (int j = 0; j < 3; j++ ) {
                Serial.println("Connecting to " + ssid);
                String password = configJson["networks"][ssid];
                WiFi.begin(WiFi.SSID(i).c_str(), password.c_str());
                delay(1000);

                if (WiFi.status() == WL_CONNECTED) {
                    break;
                }
            }
            
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println(WiFi.localIP());
            break;
        }
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Unable to connect to WiFi, starting access point");
        usingAP = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ap_ssid, ap_password); 
    } 

    if (!MDNS.begin("kneebrace")) {
        Serial.println("Error starting mDNS");
    } else {
        Serial.println("Started mDNS, visit `kneebrace.local` in browser");
    }
}

unsigned long networkCurrentMillis = millis();
unsigned long networkPreviousMillis = millis();
unsigned long networkInterval = 30000;

void networkLoop() {
    networkCurrentMillis = millis();
    if (!usingAP && (WiFi.status() != WL_CONNECTED) && (networkCurrentMillis - networkPreviousMillis >=networkInterval)) {
        Serial.println("Reconnecting to WiFi...");
        networkSetup();
        networkPreviousMillis = networkCurrentMillis;
    }
}
