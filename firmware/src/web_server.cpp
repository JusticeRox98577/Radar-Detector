#include "web_server.h"
#include "config.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

static AsyncWebServer _serverInst(80);
static AsyncWebSocket _wsInst("/ws");

void RadarWebServer::begin() {
    _server = &_serverInst;
    _ws     = &_wsInst;

    if (!SPIFFS.begin(true)) {
        Serial.println("[WEB] SPIFFS mount failed");
        return;
    }

    _ws->onEvent([](AsyncWebSocket*, AsyncWebSocketClient* client,
                    AwsEventType type, void*, uint8_t*, size_t) {
        if (type == WS_EVT_CONNECT)
            Serial.printf("[WS] client #%u connected\n", client->id());
        else if (type == WS_EVT_DISCONNECT)
            Serial.printf("[WS] client #%u disconnected\n", client->id());
    });
    _server->addHandler(_ws);

    _setupRoutes();
    _server->begin();
    Serial.println("[WEB] server started on port 80");
}

void RadarWebServer::_setupRoutes() {
    _server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    _server->onNotFound([](AsyncWebServerRequest* req) {
        req->send(404, "text/plain", "Not found");
    });
}

void RadarWebServer::broadcast(const LaserReading& r, float speedKph) {
    if (_ws->count() == 0) return;

    auto sideStr = [](LaserSide s) -> const char* {
        switch (s) {
            case LaserSide::FRONT: return "FRONT";
            case LaserSide::REAR:  return "REAR";
            case LaserSide::LEFT:  return "LEFT";
            case LaserSide::RIGHT: return "RIGHT";
            default:               return "NONE";
        }
    };

    JsonDocument doc;
    doc["ts"]     = r.timestampMs;
    doc["alert"]  = r.alert;
    doc["side"]   = sideStr(r.side);
    doc["prf_ms"] = r.prf_ms;
    doc["pulses"] = r.pulseCount;
    doc["speed"]  = speedKph;

    String payload;
    serializeJson(doc, payload);
    _ws->textAll(payload);
}
