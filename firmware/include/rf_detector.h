#pragma once
#include <Arduino.h>

enum class RadarBand : uint8_t {
    NONE = 0,
    X    = 1,   // 10.525 GHz  (HB100)
    K    = 2,   // 24.125 GHz  (CDM324)
};

enum class Direction : uint8_t {
    NONE  = 0,
    FRONT = 1,
    REAR  = 2,
    BOTH  = 3
};

struct BandReading {
    uint16_t  frontRaw;
    uint16_t  rearRaw;
    int16_t   frontAboveNoise;
    int16_t   rearAboveNoise;
    Direction direction;
    uint8_t   strength;    // 0–10 normalized signal strength
};

struct RFReading {
    BandReading x;
    BandReading k;
    RadarBand   dominantBand;   // highest-priority active band
    uint32_t    timestampMs;
};

class RFDetector {
public:
    void      begin();
    RFReading sample();

    uint16_t xFrontBaseline() const { return _xFrontBase; }
    uint16_t xRearBaseline()  const { return _xRearBase;  }
    uint16_t kFrontBaseline() const { return _kFrontBase; }
    uint16_t kRearBaseline()  const { return _kRearBase;  }

private:
    uint16_t   _readADC(uint8_t pin) const;
    BandReading _processBand(uint8_t frontPin, uint8_t rearPin,
                              uint16_t& frontBase, uint16_t& rearBase,
                              uint16_t  threshold,
                              uint16_t* frontBuf, uint16_t* rearBuf) const;
    void       _updateBase(uint16_t raw, int16_t aboveNoise,
                            uint16_t threshold, uint16_t* buf,
                            uint8_t idx, uint8_t count,
                            uint16_t& baseline) const;

    static constexpr uint8_t BUF = 64;

    uint16_t _xFrontBuf[BUF] = {};  uint16_t _xRearBuf[BUF] = {};
    uint16_t _kFrontBuf[BUF] = {};  uint16_t _kRearBuf[BUF] = {};
    uint8_t  _bufIdx  = 0;
    bool     _bufFull = false;

    uint16_t _xFrontBase = 0;  uint16_t _xRearBase = 0;
    uint16_t _kFrontBase = 0;  uint16_t _kRearBase = 0;
};
