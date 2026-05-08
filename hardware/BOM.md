# Bill of Materials — DIY Laser Detector

All parts are readily available on Amazon.

---

## Parts List

| Qty | Part | Amazon search term | Approx price |
|-----|------|--------------------|--------------|
| 1   | ESP32 DevKit v1 (38-pin) | "ESP32 DevKitC 38 pin" | $8–12 |
| 4–8 | TEPT5700 or TEKT5400S photodiode (wide-angle, 904 nm sensitive) | "TEPT5700 photodiode" or "IR photodiode 5mm 940nm" | $5–8 (pack) |
| 2   | LM358 dual op-amp DIP-8 | "LM358N op-amp" | $5 (10-pack) |
| 2   | LM393 dual comparator DIP-8 | "LM393 comparator DIP" | $5 (10-pack) |
| 1   | SSD1306 0.96" OLED (I2C, 128×64) | "SSD1306 OLED 0.96 inch I2C" | $5–8 |
| 1   | NEO-6M GPS module with antenna | "NEO-6M GPS module Arduino" | $10–15 |
| 1   | Active piezo buzzer (3–5 V) | "active buzzer module Arduino" | $5 (pack) |
| 2   | 5 mm red LED | "5mm red LED" | <$2 (pack) |
| 4   | 100 Ω resistor | "100 ohm resistor" | <$2 |
| 4   | 10 kΩ resistor | "10k resistor" | <$2 |
| 4   | 100 kΩ resistor | "100k resistor" | <$2 |
| 4   | 0.1 µF ceramic capacitor | "0.1uf ceramic capacitor" | <$2 |
| 1   | 100 µF electrolytic capacitor (power rail decoupling) | "100uf electrolytic capacitor" | <$2 |
| 1   | Perfboard or solderless breadboard | "400 point breadboard" | $5 |
| 1   | ABS plastic enclosure (~10×6×3 cm) | "small ABS project box" | $6–10 |
| 1   | USB-C power bank or 5 V car USB charger | — | — |

**Estimated total: $60–90 USD**

---

## Ka-Band Note

**Ka-band (33.4–36 GHz) cannot be detected with Amazon-available parts.**

Genuine Ka-band radar detection requires a superheterodyne receiver front-end
operating at millimetre-wave frequencies — specifically an LNA + mixer + local
oscillator at ~35 GHz, plus a microstrip PCB with controlled-impedance traces.
These components are only available from RF distributors (Mouser, DigiKey) as
individual MMIC chips and require professional PCB fabrication.
This is beyond the scope of a breadboard/perfboard DIY project.

Commercial detectors like the Uniden R7 use custom multi-band ASICs.
No hobbyist-accessible Ka-band module exists.

**What this detector does cover:**
- ✅ **Laser / LIDAR** (904 nm) — the most common and directional threat
- ❌ Ka-band — not supported on any DIY platform

---

## Laser Sensor Signal Chain

Police LIDAR fires ~33 ns pulses at 904 nm.  Each pulse is too short to sample
with an ADC, so the circuit converts it to a digital edge instead:

```
Photodiode (TEPT5700)
    │
    ├── 10 kΩ load resistor to 3.3 V     ← reverse-biased PD in photovoltaic mode
    │
    └──► TIA stage (LM358):
             non-inv (+) ── mid-rail bias (two 10 kΩ in voltage divider)
             feedback    ── 100 kΩ
             output      ── ~0–3 V analog signal
                │
                └──► Comparator (LM393):
                         (+) ── TIA output
                         (−) ── threshold pot (10 kΩ trim) or fixed ~1.5 V
                         out ── 3.3 kΩ pull-up to 3.3 V → GPIO (digital edge)
```

Each detected laser pulse produces a rising edge on the GPIO pin.
The firmware times the interval between edges to verify PRF (50–333 Hz = valid
police laser) before alerting.

**Sensitivity tips:**
- Use 4 photodiodes for 360° coverage (front, rear, left, right).
- Angle each PD slightly outward (~15°) for wider field of view.
- Keep the comparator threshold just above the ambient noise floor — adjust
  the trim pot in bright sunlight to avoid false triggers from sun glints.
