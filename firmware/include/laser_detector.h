#pragma once
#include <Arduino.h>

enum class LaserSide : uint8_t {
    NONE  = 0,
    FRONT = 1,
    REAR  = 2,
    LEFT  = 3,
    RIGHT = 4
};

struct LaserReading {
    bool       alert;
    LaserSide  side;
    uint16_t   prf_ms;      // measured inter-pulse interval
    uint8_t    pulseCount;  // pulses in current burst
    uint32_t   timestampMs;
};

class LaserDetector {
public:
    void        begin();
    LaserReading poll();    // call every loop; processes ISR data

    // ISR-safe callbacks (called from IRAM interrupt handlers)
    static void IRAM_ATTR isrFront();
    static void IRAM_ATTR isrRear();
    static void IRAM_ATTR isrLeft();
    static void IRAM_ATTR isrRight();

private:
    struct SideState {
        volatile uint32_t lastPulseUs = 0;
        volatile uint32_t pulseIntervalUs = 0;
        volatile uint8_t  rawCount = 0;    // total pulses since last reset
        volatile bool     newData  = false;
    };

    static SideState _sides[4];

    LaserReading _evaluate(uint8_t idx, LaserSide side) const;
    void         _resetSide(uint8_t idx);

    bool     _alertActive  = false;
    uint32_t _alertStartMs = 0;
    LaserSide _alertSide   = LaserSide::NONE;
};
