# Automated Metabolic Cage with Inline Electrochemical Urine Analysis

A fully standalone, automated system for pooled urine collection and inline electrochemical ionic analysis in laboratory rodents — eliminating manual sample handling, external laboratory analysis, and dependency on connected computers.

---

## Overview

This system integrates ion-selective electrode (ISE) potentiometry directly within the metabolic cage urine collection pathway. A single push-button triggers gravitational sample transfer into a miniaturized sensing chamber, where pH, Na⁺, and Cl⁻ are simultaneously measured using the Nernst equation implemented in onboard firmware. Results are displayed directly on an LCD screen with no external software required.

> **Patent Status:** Invention disclosure filed  
> **Application Area:** Renal Physiology / Pharmacology / Nephrotoxicity Screening

---

## How It Works

Urine produced over a 5-hour collection period pools in a calibrated beaker sealed by a normally-closed solenoid valve. On operator button press:

1. Solenoid valve opens (relay on D4) → urine flows into sensing chamber
2. Four electrodes (pH, Na⁺, Cl⁻, Reference) measure simultaneously
3. Arduino averages 10 ADC samples per sensor
4. Reference voltage subtracted + voltage divider compensation applied
5. Nernst equation calculates ionic concentrations
6. Results displayed on LCD with physiological range status
7. Valve closes automatically

```
Nernst equation (Na⁺, Cl⁻):  concentration = 10 ^ ((E_cell - E0) / 0.05916)
Nernst equation (pH):          pH = (E0 - E_cell) / 0.05916
```

---

## Hardware

| Component | Quantity | Purpose |
|---|---|---|
| Arduino Uno | 1 | Microcontroller |
| Ion-Selective Electrode — pH | 1 | Urinary pH measurement |
| Ion-Selective Electrode — Na⁺ | 1 | Sodium concentration |
| Ag/AgCl Electrode — Cl⁻ | 1 | Chloride concentration |
| Ag/AgCl Reference Electrode | 1 | Stable electrochemical baseline |
| Solenoid Valve (normally closed) | 1 | Sample transfer control |
| Relay Module (SPDT) | 1 | Valve actuation via D4 |
| LCD 16×2 | 1 | Results display |
| Push Button | 1 | Measurement trigger (D2) |
| Resistors (3kΩ, 15kΩ, 10kΩ, 100Ω) | 1 set | Signal conditioning / voltage dividers |
| Acrylic sensing chamber | 1 | Flow-through electrode housing |

> **Note:** Tinkercad simulation uses potentiometers as ISE stand-ins and LCD 16×2 instead of TFT display.

**Pin Mapping:**

| Sensor | Arduino Pin | Voltage Divider |
|---|---|---|
| pH Electrode | A0 | 3kΩ / 15kΩ (ratio 0.8333) |
| Na⁺ ISE | A1 | 10kΩ impedance match |
| Cl⁻ Electrode | A2 | 3kΩ / 15kΩ (ratio 0.8333) |
| Reference Electrode | A3 | 100Ω direct |
| Push Button | D2 | INPUT_PULLUP |
| Solenoid Relay | D4 | Digital output |
| LCD (RS, EN, D4–D7) | D8–D13 | SPI |

---

## Project Structure

```
├── metabolic_cage_tinkercad.ino    # Arduino firmware
├── circuit_diagram.png             # Tinkercad circuit diagram
└── README.md
```

---

## Getting Started

### Arduino Setup
1. Open `metabolic_cage_tinkercad.ino` in Arduino IDE
2. Install library: **LiquidCrystal** (built into Arduino IDE)
3. **Update calibration constants** at the top of the file:
```cpp
float E0_pH = 0.420;   // Determine from pH 4, 7, 10 buffer calibration
float E0_Na = 0.320;   // Determine from 10, 50, 100, 200 mM NaCl standards
float E0_Cl = 0.280;   // Determine from KCl/NaCl concentration series
```
4. Select **Board:** Arduino Uno, **Port:** your COM port
5. Upload to Arduino

### Running a Measurement
1. Allow rodent 5-hour urine collection period
2. Read urine volume from calibrated beaker
3. Press **MEASURE button** (D2)
4. Wait ~4 seconds for valve cycle + stabilisation
5. Read results on LCD display
6. Flush chamber with distilled water (30–60 seconds)

---

## Output

**LCD Display:**
```
pH:6.82  Na:142
Cl:118mM   OK
```

**Serial Monitor output:**
```
CAGE_READY
MEASURE_START
pH:6.82,Na:142.1,Cl:118.3,Range:OK
MEASURE_END
```

**Analytical metrics provided:**

| Metric | Unit | Normal Range (Rodent) |
|---|---|---|
| Urinary pH | — | 4.5 – 8.5 |
| Sodium (Na⁺) | mM | 20 – 300 mM |
| Chloride (Cl⁻) | mM | 20 – 350 mM |

Total ionic excretion = displayed concentration × urine volume (read from beaker)

---

## Key Technical Decisions

**Pooled collection methodology** — consistent with gold-standard clinical urine analysis protocols representing total renal output over the experimental period.

**Nernst equation in firmware** — concentration calculations run entirely onboard, eliminating dependency on external computers or software.

**10-sample ADC averaging** — reduces thermal noise, quantization error, and random electrical noise before Nernst calculation.

**Reference electrode subtraction** — differential measurement eliminates ground loop interference and temperature-dependent drift common to single-ended analog measurements.

**Gravity-driven passive fluid transport** — no pumps or compressors, eliminating mechanical failure modes.

---

## Sensor Calibration

Before each experiment, calibrate the three ISEs and update `E0` values in firmware:

- **pH:** Use pH 4.0, 7.0, 10.0 buffer solutions → plot voltage vs pH → y-intercept = E0_pH
- **Na⁺:** Use 10, 50, 100, 200 mM NaCl → plot voltage vs log₁₀(concentration) → y-intercept = E0_Na
- **Cl⁻:** Use same NaCl/KCl series → plot voltage vs log₁₀(concentration) → y-intercept = E0_Cl

Expected Nernst slope ≈ **59.16 mV per decade** at 25°C.

---

## Citation

If you use this system in your research, please cite:

> Automated Metabolic Cage with Inline Electrochemical Urine Analysis System. Invention disclosure filed, 2024.

---

## 🙋 Author

Developed as part of open-architecture renal physiology instrumentation research.
