// Replace with your network credentials
char *ssid = "RAHULXPS 6734";
char *password = "K54@8m03";

char *ap_ssid = "kneebrace";
char *ap_password = "smartorthotics!";

void networkSetup(void) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println(WiFi.localIP());

    /*
    Serial.println("Starting access point");
    WiFi.softAP(ap_ssid, ap_password); 
    */
}
