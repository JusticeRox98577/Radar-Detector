#pragma once
#include <Arduino.h>
#include "laser_detector.h"

class AsyncWebServer;
class AsyncWebSocket;

class RadarWebServer {
public:
    void begin();
    void broadcast(const LaserReading& r, float speedKph);

private:
    void _setupRoutes();
    AsyncWebServer* _server = nullptr;
    AsyncWebSocket* _ws     = nullptr;
};
