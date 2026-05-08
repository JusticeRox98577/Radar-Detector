# DIY Laser Detector

A laser/LIDAR detector built around an ESP32, inspired by the Uniden R7.  
Detects police laser (904 nm) from four directions (front, rear, left, right),
shows alerts on an OLED display, logs current speed via GPS, and streams live
data to a web dashboard over WiFi.

## What it detects

| Threat | Supported | Hardware |
|--------|-----------|----------|
| Laser / LIDAR (904 nm) | ✅ Yes | TEPT5700 photodiode + LM358 TIA + LM393 comparator |
| Ka-band (33–36 GHz) | ❌ No | Requires mm-wave MMIC chips — not available on Amazon |

## Feature overview (R7-inspired)

- **4-direction laser detection** — front, rear, left, right photodiode arrays
- **Pulse validation** — only alerts on 50–333 Hz PRF bursts (rejects sun glints)
- **OLED display** — shows band, direction arrow, live speed, mute indicator
- **GPS speed readout** — NEO-6M module via UART2
- **Web dashboard** — connect phone/laptop to the device WiFi AP and open
  `http://192.168.4.1` for a live display with alert log
- **Mute button** — silences buzzer without dismissing the visual alert
- **Alert hold** — alert stays displayed for 4 seconds after signal disappears

---

## Hardware

See [`hardware/BOM.md`](hardware/BOM.md) for the full parts list (~$60–90 on Amazon)  
See [`hardware/wiring.md`](hardware/wiring.md) for wiring and per-sensor circuit schematic

**Summary:**

| Part | Role |
|------|------|
| ESP32 DevKit v1 | Main controller, WiFi, GPIO interrupts |
| TEPT5700 ×4–8 | 904 nm photodiodes, one array per direction |
| LM358 ×2 | Transimpedance amplifier (TIA) for each photodiode |
| LM393 ×2 | Fast comparator — converts analog pulse to digital edge |
| SSD1306 OLED | 128×64 I2C display |
| NEO-6M GPS | Speed and position via UART2 |
| Piezo buzzer | Audible alert |
| 10 kΩ trim pot ×4 | Per-sensor threshold calibration |

---

## Firmware

**Toolchain:** [PlatformIO](https://platformio.org/) (VS Code extension or CLI)

### Install

```bash
# Install PlatformIO CLI
pip install platformio

# Or use the VS Code extension: marketplace.visualstudio.com/items?itemName=platformio.platformio-ide
```

### Build and flash

```bash
cd firmware

# Build firmware only
pio run

# Upload firmware to ESP32 (adjust port if needed)
pio run --target upload --upload-port /dev/ttyUSB0   # Linux
pio run --target upload --upload-port COM3            # Windows

# Upload web UI to SPIFFS filesystem
pio run --target uploadfs --upload-port /dev/ttyUSB0

# Open serial monitor
pio device monitor --baud 115200
```

> Flash the firmware **first**, then flash the filesystem (`uploadfs`).

### Calibration

After first flash, open the serial monitor. You should see:
```
[MAIN] ready — connect to WiFi and open http://192.168.4.1
```

**Threshold calibration:**
1. Power the device in normal ambient light (no laser sources).
2. Adjust each sensor's 10 kΩ trim pot until the LM393 output stays LOW
   (no false triggers on the serial monitor or LED).
3. Gradually turn the pot toward lower threshold until it just stops triggering
   on ambient light — that's your operating point.
4. Point a TV/AV remote at each sensor and press a button to verify the circuit
   detects 940 nm pulses (close enough to 904 nm for bench testing).

---

## Web Dashboard

1. Power the device.
2. Connect your phone or laptop to WiFi SSID `RadarDetector` (password: `detect1234`).
3. Open `http://192.168.4.1` in a browser.

The dashboard updates at 10 Hz via WebSocket and shows:
- Large LASER alert label with direction
- 4-point compass rose lighting up the detected direction
- Pulse rate (Hz) and pulse count
- Current speed from GPS
- Scrollable alert log with timestamps

---

## Project structure

```
firmware/
  src/
    main.cpp            — setup/loop, alert state machine, buzzer/LED
    laser_detector.cpp  — ISR-based pulse timing, PRF validation
    web_server.cpp      — AsyncWebServer + WebSocket JSON broadcast
    display.cpp         — SSD1306 OLED rendering
    gps_reader.cpp      — NMEA GPRMC parser via UART2
  include/
    config.h            — all pin assignments and tuning constants
    laser_detector.h
    web_server.h
    display.h
    gps_reader.h
  data/
    index.html          — web dashboard (served from SPIFFS)
  platformio.ini
  partitions.csv

hardware/
  BOM.md               — parts list with Amazon search terms
  wiring.md            — wiring diagram and per-sensor circuit
```

---

## Limitations and notes

- **Range**: Expect roughly 100–300 m of detection range depending on photodiode
  placement and the officer's aim. The R7 achieves ~500 m+ with its multi-array
  lens system. More/larger photodiodes = more range.
- **False alerts**: Sun glints on chrome, LED taillights, and some traffic sensors
  can trigger false alerts. The PRF filter (requires 3 pulses at consistent
  interval) greatly reduces these but doesn't eliminate them.
- **Legal**: Check local laws. In some jurisdictions, possessing or operating a
  radar/laser detector is illegal. This project is for educational purposes.
