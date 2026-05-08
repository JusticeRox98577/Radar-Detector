#include "laser_detector.h"
#include "config.h"

LaserDetector::SideState LaserDetector::_sides[4];

// ─── ISR handlers (IRAM — no heap, no Serial) ────────────────────────────────

void IRAM_ATTR LaserDetector::isrFront() {
    uint32_t now = micros();
    uint32_t interval = now - _sides[0].lastPulseUs;
    _sides[0].lastPulseUs = now;
    if (interval > 0 && interval < 500000UL) {   // ignore first pulse (no prev)
        _sides[0].pulseIntervalUs = interval;
    }
    _sides[0].rawCount++;
    _sides[0].newData = true;
}
void IRAM_ATTR LaserDetector::isrRear() {
    uint32_t now = micros();
    uint32_t interval = now - _sides[1].lastPulseUs;
    _sides[1].lastPulseUs = now;
    if (interval > 0 && interval < 500000UL) _sides[1].pulseIntervalUs = interval;
    _sides[1].rawCount++;
    _sides[1].newData = true;
}
void IRAM_ATTR LaserDetector::isrLeft() {
    uint32_t now = micros();
    uint32_t interval = now - _sides[2].lastPulseUs;
    _sides[2].lastPulseUs = now;
    if (interval > 0 && interval < 500000UL) _sides[2].pulseIntervalUs = interval;
    _sides[2].rawCount++;
    _sides[2].newData = true;
}
void IRAM_ATTR LaserDetector::isrRight() {
    uint32_t now = micros();
    uint32_t interval = now - _sides[3].lastPulseUs;
    _sides[3].lastPulseUs = now;
    if (interval > 0 && interval < 500000UL) _sides[3].pulseIntervalUs = interval;
    _sides[3].rawCount++;
    _sides[3].newData = true;
}

// ─── Public interface ─────────────────────────────────────────────────────────

void LaserDetector::begin() {
    pinMode(PIN_LASER_FRONT, INPUT);
    pinMode(PIN_LASER_REAR,  INPUT);
    pinMode(PIN_LASER_LEFT,  INPUT);
    pinMode(PIN_LASER_RIGHT, INPUT);

    attachInterrupt(digitalPinToInterrupt(PIN_LASER_FRONT), isrFront, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_LASER_REAR),  isrRear,  RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_LASER_LEFT),  isrLeft,  RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_LASER_RIGHT), isrRight, RISING);
}

LaserReading LaserDetector::poll() {
    const LaserSide sides[4] = { LaserSide::FRONT, LaserSide::REAR,
                                  LaserSide::LEFT,  LaserSide::RIGHT };

    LaserReading best = { false, LaserSide::NONE, 0, 0, (uint32_t)millis() };

    for (uint8_t i = 0; i < 4; i++) {
        if (!_sides[i].newData) continue;
        _sides[i].newData = false;

        LaserReading r = _evaluate(i, sides[i]);
        if (r.alert) {
            best = r;
            break;  // highest-priority: first hit wins
        }
    }

    // Sustain alert for LASER_ALERT_HOLD_MS
    uint32_t now = millis();
    if (best.alert) {
        _alertActive  = true;
        _alertStartMs = now;
        _alertSide    = best.side;
    }

    if (_alertActive && (now - _alertStartMs) >= LASER_ALERT_HOLD_MS) {
        _alertActive = false;
        _alertSide   = LaserSide::NONE;
        // Reset pulse counters so a new burst can re-trigger
        for (uint8_t i = 0; i < 4; i++) _resetSide(i);
    }

    if (_alertActive && !best.alert) {
        // sustain previous alert
        best.alert  = true;
        best.side   = _alertSide;
    }

    return best;
}

// ─── Private ──────────────────────────────────────────────────────────────────

LaserReading LaserDetector::_evaluate(uint8_t idx, LaserSide side) const {
    // Snapshot ISR data atomically (disable IRQ briefly)
    noInterrupts();
    uint32_t intervalUs = _sides[idx].pulseIntervalUs;
    uint8_t  count      = _sides[idx].rawCount;
    interrupts();

    uint32_t intervalMs = intervalUs / 1000UL;

    bool prfOk = (intervalMs >= LASER_PRF_MIN_MS &&
                  intervalMs <= LASER_PRF_MAX_MS);
    bool alert = prfOk && (count >= LASER_N_MIN_PULSES);

    return { alert, side, (uint16_t)intervalMs, count,
             (uint32_t)millis() };
}

void LaserDetector::_resetSide(uint8_t idx) {
    noInterrupts();
    _sides[idx].rawCount         = 0;
    _sides[idx].pulseIntervalUs  = 0;
    interrupts();
}
