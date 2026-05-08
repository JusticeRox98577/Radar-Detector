#include "display.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

void Display::begin() {
    Wire.begin(PIN_SDA, PIN_SCL);
    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        Serial.println("[OLED] init failed");
        return;
    }
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(16, 24);
    oled.print("LASER DETECTOR");
    oled.setCursor(28, 40);
    oled.print("Starting...");
    oled.display();
}

void Display::update(const LaserReading& r, float speedKph, bool muted) {
    oled.clearDisplay();
    if (!r.alert) {
        _drawIdle(speedKph);
    } else {
        _drawAlert(r, speedKph, muted);
    }
    oled.display();
}

void Display::_drawIdle(float speedKph) {
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("CLEAR");

    oled.setTextSize(3);
    char buf[6];
    snprintf(buf, sizeof(buf), "%3d", (int)speedKph);
    oled.setCursor(28, 20);
    oled.print(buf);

    oled.setTextSize(1);
    oled.setCursor(50, 54);
    oled.print("km/h");
}

void Display::_drawAlert(const LaserReading& r, float speedKph, bool muted) {
    // "LASER" in large text
    oled.setTextSize(2);
    oled.setCursor(0, 0);
    oled.print("! LASER !");

    // Direction
    oled.setTextSize(2);
    oled.setCursor(0, 22);
    switch (r.side) {
        case LaserSide::FRONT: oled.print(">> FRONT"); break;
        case LaserSide::REAR:  oled.print("<< REAR "); break;
        case LaserSide::LEFT:  oled.print("^ LEFT  "); break;
        case LaserSide::RIGHT: oled.print("v RIGHT "); break;
        default:               oled.print("        "); break;
    }

    // PRF + speed on bottom line
    oled.setTextSize(1);
    char buf[32];
    snprintf(buf, sizeof(buf), "%dHz  %dkm/h%s",
             r.prf_ms > 0 ? (int)(1000 / r.prf_ms) : 0,
             (int)speedKph,
             muted ? " MUTE" : "");
    oled.setCursor(0, 54);
    oled.print(buf);
}
