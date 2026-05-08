#pragma once

// ─── Laser / LIDAR Detection ──────────────────────────────────────────────────
//
//  Police laser (LIDAR) fires 904 nm pulses.
//  Each photodiode + TIA + LM393 comparator produces a 3.3 V digital pulse
//  on every received laser pulse.  ESP32 GPIO interrupts time the pulses.
//
//  Typical police laser PRF: 100–200 Hz  (4–10 ms between pulses)
//  Pulse width:              ~33–200 ns  (too short to measure, just detect edge)
//
// ─── Laser sensor pins (comparator digital output → ESP32 GPIO) ───────────────
//  Use GPIO-interrupt-capable pins only (any input-capable GPIO on ESP32).
#define PIN_LASER_FRONT   4    // front photodiode array
#define PIN_LASER_REAR    5    // rear  photodiode array
#define PIN_LASER_LEFT    18   // left  (optional; wire to FRONT if not fitted)
#define PIN_LASER_RIGHT   19   // right (optional; wire to FRONT if not fitted)

// ─── Alert outputs ────────────────────────────────────────────────────────────
#define PIN_BUZZER        25
#define PIN_LED_LASER     26   // red LED  – laser alert
#define PIN_LED_STATUS    2    // onboard  – WiFi ready
#define PIN_BTN_MUTE      0    // active-low mute (boot button on DevKit)

// ─── OLED (SSD1306 I2C) ──────────────────────────────────────────────────────
#define PIN_SDA           21
#define PIN_SCL           22
#define OLED_I2C_ADDR     0x3C
#define OLED_WIDTH        128
#define OLED_HEIGHT       64

// ─── GPS (NEO-6M UART2) ──────────────────────────────────────────────────────
#define PIN_GPS_RX        16
#define PIN_GPS_TX        17
#define GPS_BAUD          9600

// ─── Laser pulse analysis ─────────────────────────────────────────────────────
// A valid police laser burst is N_MIN_PULSES pulses with inter-pulse interval
// between PRF_MIN_MS and PRF_MAX_MS.  Sun glint or electrical noise tend to
// produce either single pulses or very irregular intervals.
#define LASER_PRF_MIN_MS   3    // 3 ms → ~333 Hz (upper limit)
#define LASER_PRF_MAX_MS  20    // 20 ms → 50 Hz  (lower limit)
#define LASER_N_MIN_PULSES 3    // require 3 consecutive valid pulses before alert
#define LASER_ALERT_HOLD_MS 4000

// ─── WiFi AP ─────────────────────────────────────────────────────────────────
#define WIFI_SSID  "RadarDetector"
#define WIFI_PASS  "detect1234"
#define WIFI_CH    6

// ─── WebSocket broadcast rate ─────────────────────────────────────────────────
#define WS_BROADCAST_INTERVAL_MS  100
