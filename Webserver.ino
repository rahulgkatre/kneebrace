// Create AsyncWebServer object on port 80
// Create WebSocket to request IMU data on
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void getNewPlotData()
{
    if (!mockBNO08X)
    {
        if (bno08x.wasReset())
        {
            Serial.print("sensor was reset ");
            setReports(reportType, reportIntervalUs);
        }

        if (bno08x.getSensorEvent(&sensorValue))
        {
            // in this demo only one report type will be received depending on FAST_MODE define (above)
            switch (sensorValue.sensorId)
            {
            case SH2_ARVR_STABILIZED_RV:
                quaternionToEulerRV(&sensorValue.un.arvrStabilizedRV, &ypr, true);
            case SH2_GYRO_INTEGRATED_RV:
                // faster (more noise?)
                quaternionToEulerGI(&sensorValue.un.gyroIntegratedRV, &ypr, true);
                break;
            }
            static long last = 0;
            long now = micros();
            // Serial.print(now - last);             Serial.print("\t");
            last = now;
            // Serial.print(sensorValue.status);     Serial.print("\t");  // This is accuracy in the range of 0 to 3
            // Serial.print(ypr.yaw);                Serial.print("\t");
            // Serial.print(ypr.pitch);              Serial.print("\t");
            // Serial.println(ypr.roll);
        }

        ws.textAll("{\"type\":\"plot\",\"labels\":[\"yaw\",\"pitch\",\"roll\"],\"data\":[" + String(ypr.yaw) + "," + String(ypr.pitch) + "," + String(ypr.roll) + "]}");
    }
    else
    {
        ws.textAll("{\"type\":\"plot\",\"labels\":[\"yaw\",\"pitch\",\"roll\"],\"data\":[" + String(random(-45, 45)) + "," + String(random(-45, 45)) + "," + String(random(-45, 45)) + "]}");
    }
}

void getNewTextData()
{
    ws.textAll("{\"type\":\"text\",\"data\":{\"test\":\"Hello, world!\"}}");
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

void webserver_setup()
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

void webserver_loop()
{
    ws.cleanupClients();
}
