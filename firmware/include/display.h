#pragma once
#include <Arduino.h>
#include "laser_detector.h"

class Display {
public:
    void begin();
    void update(const LaserReading& r, float speedKph, bool muted);

private:
    void _drawIdle(float speedKph);
    void _drawAlert(const LaserReading& r, float speedKph, bool muted);
};
