#pragma once
#include <Arduino.h>

struct GPSData {
    float    speedKph;
    float    lat;
    float    lng;
    bool     valid;
};

class GPSReader {
public:
    void    begin();
    void    poll();          // call every loop iteration
    GPSData data() const { return _data; }

private:
    GPSData _data = { 0.f, 0.f, 0.f, false };

    // NMEA parser state
    char    _buf[96]  = {};
    uint8_t _pos      = 0;
    void    _parseLine(const char* line);
    bool    _parseGPRMC(const char* line);
};
