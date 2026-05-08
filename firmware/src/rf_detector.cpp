#include "rf_detector.h"
#include "config.h"
#include <algorithm>

void RFDetector::begin() {
    analogReadResolution(ADC_RESOLUTION);
    analogSetAttenuation(ADC_11db);

    // Prime noise-floor buffers with quiet readings before WiFi starts
    for (uint8_t i = 0; i < BUF; i++) {
        _xFrontBuf[i] = _readADC(PIN_X_FRONT_ADC);
        _xRearBuf[i]  = _readADC(PIN_X_REAR_ADC);
        _kFrontBuf[i] = _readADC(PIN_K_FRONT_ADC);
        _kRearBuf[i]  = _readADC(PIN_K_REAR_ADC);
        delay(5);
    }
    _bufFull = true;

    // Calculate initial baselines from buffer averages
    auto avg = [](uint16_t* buf, uint8_t n) -> uint16_t {
        uint32_t s = 0;
        for (uint8_t i = 0; i < n; i++) s += buf[i];
        return s / n;
    };
    _xFrontBase = avg(_xFrontBuf, BUF);
    _xRearBase  = avg(_xRearBuf,  BUF);
    _kFrontBase = avg(_kFrontBuf, BUF);
    _kRearBase  = avg(_kRearBuf,  BUF);
}

RFReading RFDetector::sample() {
    RFReading r;
    r.timestampMs = millis();

    r.x = _processBand(PIN_X_FRONT_ADC, PIN_X_REAR_ADC,
                        _xFrontBase, _xRearBase, X_BAND_THRESHOLD,
                        _xFrontBuf, _xRearBuf);
    r.k = _processBand(PIN_K_FRONT_ADC, PIN_K_REAR_ADC,
                        _kFrontBase, _kRearBase, K_BAND_THRESHOLD,
                        _kFrontBuf, _kRearBuf);

    // Priority: K > X (Ka would sit between them if added later)
    if (r.k.direction != Direction::NONE)      r.dominantBand = RadarBand::K;
    else if (r.x.direction != Direction::NONE) r.dominantBand = RadarBand::X;
    else                                        r.dominantBand = RadarBand::NONE;

    _bufIdx = (_bufIdx + 1) % BUF;
    if (_bufIdx == 0) _bufFull = true;

    return r;
}

BandReading RFDetector::_processBand(uint8_t frontPin, uint8_t rearPin,
                                      uint16_t& frontBase, uint16_t& rearBase,
                                      uint16_t threshold,
                                      uint16_t* frontBuf, uint16_t* rearBuf) const {
    BandReading b;
    b.frontRaw = _readADC(frontPin);
    b.rearRaw  = _readADC(rearPin);
    b.frontAboveNoise = (int16_t)b.frontRaw - (int16_t)frontBase;
    b.rearAboveNoise  = (int16_t)b.rearRaw  - (int16_t)rearBase;

    bool frontHit = b.frontAboveNoise > (int16_t)threshold;
    bool rearHit  = b.rearAboveNoise  > (int16_t)threshold;

    if      (frontHit && rearHit) b.direction = Direction::BOTH;
    else if (frontHit)            b.direction = Direction::FRONT;
    else if (rearHit)             b.direction = Direction::REAR;
    else                          b.direction = Direction::NONE;

    // Normalize strength 0–10 from whichever side is stronger
    int16_t peak = std::max(b.frontAboveNoise, b.rearAboveNoise);
    peak = std::max(peak, (int16_t)0);
    // Clamp to 3× threshold = full scale
    b.strength = (uint8_t)std::min(10, (int)(peak * 10) / (threshold * 3));

    // Only update the noise floor from quiet samples
    uint8_t count = _bufFull ? BUF : _bufIdx;
    _updateBase(b.frontRaw, b.frontAboveNoise, threshold,
                frontBuf, _bufIdx, count, frontBase);
    _updateBase(b.rearRaw,  b.rearAboveNoise,  threshold,
                rearBuf,  _bufIdx, count, rearBase);

    return b;
}

void RFDetector::_updateBase(uint16_t raw, int16_t aboveNoise,
                              uint16_t threshold, uint16_t* buf,
                              uint8_t idx, uint8_t count,
                              uint16_t& baseline) const {
    if (aboveNoise <= (int16_t)threshold) {
        buf[idx] = raw;
    }
    if (count == 0) return;
    uint32_t s = 0;
    for (uint8_t i = 0; i < count; i++) s += buf[i];
    baseline = s / count;
}

uint16_t RFDetector::_readADC(uint8_t pin) const {
    uint32_t acc = 0;
    for (uint8_t i = 0; i < ADC_SAMPLES; i++) acc += analogRead(pin);
    return (uint16_t)(acc / ADC_SAMPLES);
}
