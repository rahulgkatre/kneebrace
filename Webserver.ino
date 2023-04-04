// Create AsyncWebServer object on port 80
// Create WebSocket to request IMU data on
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void getNewPlotData()
{
    String message;
    if (!mockBNO08X)
    {
        String accelJson = getAccelJsonString();
        String gyroJson = getGyroJsonString();
        String magFieldJson = getMagFieldJsonString();
        String eulerJson = getEulerJsonString();
        message = "{\"type\":\"plot\",\"series\":[" + accelJson + "," + gyroJson + "," + magFieldJson + ","  + eulerJson + "]}";
    }
    else
    {
        String mockJson = getMockJsonString();
        message = "{\"type\":\"plot\",\"series\":" + mockJson + "}";
    }
    ws.textAll(message);
}

void getNewTextData()
{
    String message;
    if (!mockBNO08X) {
        String stepsJson = getStepsJsonString();
        String stabilityJson = getStabilityJsonString();
        String activityJson = getActivityJsonString();
        message = "{\"type\":\"text\",\"series\":[" + stepsJson + "," + stabilityJson + "," + activityJson + "]}";
    } else {
        message = "{\"type\":\"text\",\"series\":[{\"label\":\"Test\",\"data\":\"Hello, world!\"}]}";
    }
    ws.textAll(message);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        if (strcmp((char *)data, "getNewPlotData") == 0)
        {
            getNewPlotData();
        }
        else if (strcmp((char *)data, "getNewTextData") == 0)
        {
            getNewTextData();
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
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

String processor(const String &var)
{
    Serial.println(var);
    return String();
}

void webServerSetup()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", String(), false, processor); });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/style.css", "text/css"); });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "text/javascript"); });

    server.on("/c3.min.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/c3.min.css", "text/css"); });

    server.on("/c3.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/c3.min.js", "text/javascript"); });

    server.on("/d3.v5.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/d3.v5.min.js"); });


    // Start server
    server.begin();
}

void webServerLoop()
{
    ws.cleanupClients();
}
