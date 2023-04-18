// Create AsyncWebServer object on port 80
// Create WebSocket to request IMU data on
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void getNewPlotData() {
    // String accelJson = getAccelJsonString();
    String gyroFilterJson = getGyroFilterJsonString();
    // String eulerJson = getEulerJsonString();
    String flexJson = getFlexJsonString();
    String message = "{\"type\":\"plot\",\"series\":[" + /*accelJson + "," +*/ gyroFilterJson + "," + /*eulerJson + "," +*/ flexJson + "]}";
    ws.textAll(message);
}

void getNewTextData() {
    String stepsJson = getStepsJsonString();
    // String activityJson = getActivityJsonString();
    String gaitJson = getGaitAnalysisJsonString();
    String limpJson = getLimpClassificationText();
    String message = "{\"type\":\"text\",\"series\":[" + stepsJson + "," + gaitJson + "," + limpJson + "]}";
    ws.textAll(message);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        if (strcmp((char *)data, "getNewPlotData") == 0) {
            getNewPlotData();
        } else if (strcmp((char *)data, "getNewTextData") == 0) {
            getNewTextData();
        } else if (strcmp((char *)data, "hapticOff") == 0) {
            hapOff();
        } else if (strcmp((char *)data, "hapticMedium") == 0) {
            hapMedium();
        } else if (strcmp((char *)data, "hapticHigh") == 0) {
            hapHigh();
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            ws.textAll("{\"type\":\"connected\"}");
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void webServerSetup() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(LittleFS, "/style.css", "text/css"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(LittleFS, "/script.js", "text/javascript"); });
    server.on("/canvasjs.min.js", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(LittleFS, "/canvasjs.min.js", "text/javascript"); });

    // Start server
    server.begin();
    Serial.println("Starting webserver");
}

void webServerLoop() {
    ws.cleanupClients();
}
