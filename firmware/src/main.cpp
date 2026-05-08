#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "laser_detector.h"
#include "web_server.h"
#include "display.h"
#include "gps_reader.h"

static LaserDetector  laser;
static RadarWebServer webSrv;
static Display        disp;
static GPSReader      gps;

static bool     _muted        = false;
static bool     _lastMuteBtn  = HIGH;
static bool     _buzzOn       = false;
static uint32_t _lastBuzzMs   = 0;
static uint32_t _lastBcastMs  = 0;
static uint32_t _lastDispMs   = 0;

// ─── Buzzer: 100 ms on / 100 ms off while alert is active ────────────────────
static void handleBuzzer(bool alert) {
    if (!alert || _muted) {
        digitalWrite(PIN_BUZZER, LOW);
        _buzzOn = false;
        return;
    }
    uint32_t now = millis();
    if (_buzzOn && now - _lastBuzzMs >= 100) {
        digitalWrite(PIN_BUZZER, LOW);
        _buzzOn = false;
        _lastBuzzMs = now;
    } else if (!_buzzOn && now - _lastBuzzMs >= 100) {
        digitalWrite(PIN_BUZZER, HIGH);
        _buzzOn = true;
        _lastBuzzMs = now;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_BUZZER,    OUTPUT);
    pinMode(PIN_LED_LASER, OUTPUT);
    pinMode(PIN_LED_STATUS,OUTPUT);
    pinMode(PIN_BTN_MUTE,  INPUT_PULLUP);

    disp.begin();

    // Attach interrupts and start laser detector
    laser.begin();

    gps.begin();

    // Start WiFi in AP mode
    WiFi.softAP(WIFI_SSID, WIFI_PASS, WIFI_CH);
    Serial.printf("[WIFI] AP started  SSID=%s  IP=%s\n",
                  WIFI_SSID, WiFi.softAPIP().toString().c_str());
    digitalWrite(PIN_LED_STATUS, HIGH);

    webSrv.begin();
    Serial.println("[MAIN] ready — connect to WiFi and open http://192.168.4.1");
}

void loop() {
    uint32_t now = millis();

    gps.poll();
    float speedKph = gps.data().valid ? gps.data().speedKph : 0.f;

    // Debounced mute toggle
    bool muteBtn = digitalRead(PIN_BTN_MUTE);
    if (muteBtn == LOW && _lastMuteBtn == HIGH) {
        _muted = !_muted;
        Serial.printf("[MAIN] mute %s\n", _muted ? "ON" : "OFF");
    }
    _lastMuteBtn = muteBtn;

    // Poll laser detector (processes ISR data, applies hold timer)
    LaserReading r = laser.poll();

    // Alert LED mirrors alert state
    digitalWrite(PIN_LED_LASER, r.alert ? HIGH : LOW);

    handleBuzzer(r.alert);

    // OLED at ~20 Hz
    if (now - _lastDispMs >= 50) {
        _lastDispMs = now;
        disp.update(r, speedKph, _muted);
    }

    // WebSocket at 10 Hz
    if (now - _lastBcastMs >= WS_BROADCAST_INTERVAL_MS) {
        _lastBcastMs = now;
        webSrv.broadcast(r, speedKph);
    }
}
