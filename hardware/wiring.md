# Wiring Diagram

```
                      ┌──────────────────────────────────────────────────────┐
                      │                  ESP32 DevKit v1                     │
                      │                                                      │
  Laser front circuit │  GPIO4  ←── LM393 comparator out (front PD array)   │
  Laser rear  circuit │  GPIO5  ←── LM393 comparator out (rear  PD array)   │
  Laser left  circuit │  GPIO18 ←── LM393 comparator out (left  PD array)   │
  Laser right circuit │  GPIO19 ←── LM393 comparator out (right PD array)   │
                      │                                                      │
  OLED SDA            │  GPIO21 (SDA) ──────────────────────── OLED SDA     │
  OLED SCL            │  GPIO22 (SCL) ──────────────────────── OLED SCL     │
                      │                                                      │
  GPS (NEO-6M TX)     │  GPIO16 (UART2 RX) ──────────────────── GPS TX      │
  GPS (NEO-6M RX)     │  GPIO17 (UART2 TX) ──────────────────── GPS RX      │
                      │                                                      │
  Buzzer              │  GPIO25 ──── 100 Ω ──── Piezo (+)                   │
  Alert LED           │  GPIO26 ──── 220 Ω ──── LED (+)                     │
  Status LED          │  GPIO2  (onboard, WiFi ready)                       │
  Mute button         │  GPIO0  ──── button ──── GND   (active-low)         │
                      │                                                      │
  3.3 V power         │  3V3 ──── OLED VCC, GPS VCC, LM393 V+, LM358 V+    │
  Ground              │  GND ──── all module GNDs, LED cathodes, buzzer −   │
                      └──────────────────────────────────────────────────────┘
```

---

## Per-Sensor Circuit (replicate ×4)

```
3.3 V ──┬── 10 kΩ ──┬── TIA in (+)   [LM358 pin 3]
        │            │
        │          [PD]  ← TEPT5700, cathode to node, anode to GND
        │            │
        │            └── 0.1 µF ── GND   (noise filter)
        │
        └── GND via 10 kΩ voltage divider ── TIA in (−) [LM358 pin 2]
                                             feedback: 100 kΩ from out to (−)

TIA out ──────────────────────────────────────── LM393 (+) [pin 5]
                                                 LM393 (−) [pin 4] ← 10 kΩ trim pot wiper
                                                                       between 3.3 V and GND
LM393 out [pin 1] ── 3.3 kΩ pull-up to 3.3 V ──────────────────── ESP32 GPIO
```

> **Important**: The LM393 output is open-collector. The 3.3 kΩ pull-up to
> 3.3 V is required. Do NOT pull up to 5 V — the ESP32 GPIO is not 5 V tolerant.

---

## Enclosure Mounting

- Mount photodiodes flush with the front face of the enclosure.
- Use clear or semi-clear ABS for the front face panel, or drill 5 mm holes and
  hot-glue the PD leads in place.
- The 904 nm wavelength passes through most opaque-looking black plastics — test
  by shining your TV remote at the PD before finalising the enclosure.
- Place front PDs pointing ~15° downward (toward road level where laser hits).
- Place rear PDs pointing ~15° upward (police often shoot from overpass or behind).
