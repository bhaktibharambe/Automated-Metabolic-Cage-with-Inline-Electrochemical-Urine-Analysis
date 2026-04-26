// ============================================================
// Automated Metabolic Cage — Inline Urine Analysis
// Arduino Uno Firmware — Tinkercad Compatible
//
// NOTE: Tinkercad does not have ISE or TFT components.
// Potentiometers simulate ISE sensor voltages.
// LCD 16x2 replaces TFT display.
//
// Circuit:
//   Potentiometer 1 (pH sim)  → middle pin → A0, outer pins → 5V & GND
//   Potentiometer 2 (Na+ sim) → middle pin → A1, outer pins → 5V & GND
//   Potentiometer 3 (Cl- sim) → middle pin → A2, outer pins → 5V & GND
//   Potentiometer 4 (Ref sim) → middle pin → A3, outer pins → 5V & GND
//   Push button               → D2 & GND  (uses INPUT_PULLUP)
//   Relay (SPDT)              → coil IN → D4, VCC → 5V, GND → GND
//   LED                       → relay NO contact (simulates solenoid valve)
//   LCD 16x2                  → VSS=GND, VDD=5V, V0=pot, RS=D8,
//                                RW=GND, E=D9, D4=D10, D5=D11,
//                                D6=D12, D7=D13, A=5V, K=GND
// ============================================================

#include <LiquidCrystal.h>

// ── LCD PINS ────────────────────────────────────────────────
// RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// ── PIN DEFINITIONS ─────────────────────────────────────────
const int BUTTON_PIN  = 2;
const int VALVE_PIN   = 4;   // Relay → LED (solenoid stand-in)

// ── ANALOG PINS ─────────────────────────────────────────────
// A0 = pH pot, A1 = Na+ pot, A2 = Cl- pot, A3 = Reference pot

// ── CONSTANTS ───────────────────────────────────────────────
const float VREF         = 5.0;
const int   ADC_RES      = 1023;
const int   ADC_SAMPLES  = 10;
const float NERNST       = 0.05916;  // Volts at 25°C

// Voltage divider ratios (same as real circuit)
const float DIV_PH_CL = 0.8333;  // 15k / (3k + 15k)
const float DIV_NA    = 1.0;
const float DIV_REF   = 1.0;

// ── CALIBRATION CONSTANTS ────────────────────────────────────
// Adjust these after calibration with known standard solutions
const float E0_pH = 0.420;
const float E0_Na = 0.320;
const float E0_Cl = 0.280;

// ── PHYSIOLOGICAL RANGES ─────────────────────────────────────
const float PH_MIN = 4.5,  PH_MAX = 8.5;
const float NA_MIN = 20.0, NA_MAX = 300.0;
const float CL_MIN = 20.0, CL_MAX = 350.0;

// ── TIMING ──────────────────────────────────────────────────
const int DEBOUNCE_MS  = 50;
const int VALVE_MS     = 3000;
const int STABILIZE_MS = 500;

// ── SETUP ───────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(VALVE_PIN,  OUTPUT);
  digitalWrite(VALVE_PIN, LOW);

  lcd.begin(16, 2);
  lcd.print("Metabolic Cage");
  lcd.setCursor(0, 1);
  lcd.print("Press MEASURE...");

  Serial.println("CAGE_READY");
}

// ── MAIN LOOP ───────────────────────────────────────────────
void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_MS);
    if (digitalRead(BUTTON_PIN) == LOW) {
      runMeasurement();
      while (digitalRead(BUTTON_PIN) == LOW) { delay(10); }
    }
  }
}

// ── MEASUREMENT CYCLE ────────────────────────────────────────
void runMeasurement() {
  // Show measuring message
  lcd.clear();
  lcd.print("Measuring...");
  lcd.setCursor(0, 1);
  lcd.print("Please wait");
  Serial.println("MEASURE_START");

  // Open valve (relay energises → LED on)
  digitalWrite(VALVE_PIN, HIGH);
  delay(VALVE_MS);
  delay(STABILIZE_MS);

  // Read 10-sample averages
  float vPH  = readAvgVoltage(A0);
  float vNa  = readAvgVoltage(A1);
  float vCl  = readAvgVoltage(A2);
  float vRef = readAvgVoltage(A3);

  // Close valve
  digitalWrite(VALVE_PIN, LOW);

  // Reference subtraction + divider compensation
  float ePH = (vPH / DIV_PH_CL) - vRef;
  float eNa = (vNa / DIV_NA)    - vRef;
  float eCl = (vCl / DIV_PH_CL) - vRef;

  // Nernst equation
  float pH  = (E0_pH - ePH) / NERNST;
  float Na  = pow(10.0, (eNa - E0_Na) / NERNST);
  float Cl  = pow(10.0, (E0_Cl - eCl) / NERNST);

  // Clamp to physical limits
  pH = constrain(pH, 0.0,  14.0);
  Na = constrain(Na, 0.0, 1000.0);
  Cl = constrain(Cl, 0.0, 1000.0);

  // Check ranges
  bool ok = (pH >= PH_MIN && pH <= PH_MAX) &&
            (Na >= NA_MIN && Na <= NA_MAX) &&
            (Cl >= CL_MIN && Cl <= CL_MAX);

  // Display on LCD (2 rows, alternate screens)
  // Screen 1: pH and Na+
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(pH, 2);
  lcd.setCursor(8, 0);
  lcd.print("Na:");
  lcd.print((int)Na);
  lcd.setCursor(0, 1);
  lcd.print("Cl:");
  lcd.print((int)Cl);
  lcd.print("mM ");
  lcd.setCursor(8, 1);
  lcd.print(ok ? "  OK  " : " WARN!");

  // Serial output
  Serial.print("pH:");    Serial.print(pH, 2);
  Serial.print(",Na:");   Serial.print(Na, 1);
  Serial.print(",Cl:");   Serial.print(Cl, 1);
  Serial.print(",Range:"); Serial.println(ok ? "OK" : "OUT");
  Serial.println("MEASURE_END");
}

// ── AVERAGED VOLTAGE READ ─────────────────────────────────────
float readAvgVoltage(int pin) {
  long sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(pin);
    delay(2);
  }
  return ((float)sum / ADC_SAMPLES * VREF) / ADC_RES;
}
