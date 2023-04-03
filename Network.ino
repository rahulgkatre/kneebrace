// Replace with your network credentials
char *ssid = "RAHULXPS 6734";
char *password = "K54@8m03";

void network_setup(void) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());
}
