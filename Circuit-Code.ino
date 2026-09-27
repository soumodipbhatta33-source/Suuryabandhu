//C++
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20, 16, 2);

// ---------- POTs ----------
const int flat1Pot = A0;
const int flat2Pot = A1;
const int flat3Pot = A2;
const int solarPot = A3;

// ---------- Buttons ----------
const int flat4Button = 2;
const int flat5Button = 3;
const int flat6Button = 4;
const int flat7Button = 5;
const int flat8Button = 9;

// ---------- 74HC595 ----------
const int dataPin = 6;
const int clockPin = 7;
const int latchPin = 8;

// ---------- Display values ----------
int load[8];
int solarPower;
int solarGiven[8];
int gridPower[8];

void setup() {

  pinMode(flat4Button, INPUT);
  pinMode(flat5Button, INPUT);
  pinMode(flat6Button, INPUT);
  pinMode(flat7Button, INPUT);
  pinMode(flat8Button, INPUT);

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SOLAR SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("INITIALIZING");

  delay(2000);
}

void loop() {

  // =====================================
  // READ FLAT LOADS
  // =====================================

  load[0] = map(analogRead(flat1Pot), 0, 1023, 0, 1000);
  load[1] = map(analogRead(flat2Pot), 0, 1023, 0, 1000);
  load[2] = map(analogRead(flat3Pot), 0, 1023, 0, 1000);

  // Button-controlled flats
  load[3] = digitalRead(flat4Button) ? 500 : 0;
  load[4] = digitalRead(flat5Button) ? 500 : 0;
  load[5] = digitalRead(flat6Button) ? 500 : 0;
  load[6] = digitalRead(flat7Button) ? 500 : 0;
  load[7] = digitalRead(flat8Button) ? 500 : 0;

  // =====================================
  // READ SOLAR GENERATION
  // =====================================

  solarPower = map(analogRead(solarPot), 0, 1023, 0, 3000);

  // =====================================
  // SOLAR ALLOCATION
  // =====================================

  int remainingSolar = solarPower;

  for (int i = 0; i < 8; i++) {

    solarGiven[i] = 0;
    gridPower[i] = load[i];

    if (remainingSolar >= load[i]) {

      // Solar can supply the entire flat
      solarGiven[i] = load[i];
      gridPower[i] = 0;

      remainingSolar -= load[i];

    } 
    else if (remainingSolar > 0) {

      // Solar can partially supply the flat
      solarGiven[i] = remainingSolar;
      gridPower[i] = load[i] - remainingSolar;

      remainingSolar = 0;
    }
  }

  // =====================================
  // CONTROL LEDs
  // LED ON = flat receives solar
  // =====================================

  byte ledState = 0;

  for (int i = 0; i < 8; i++) {

    if (solarGiven[i] > 0) {
      ledState |= (1 << i);
    }
  }

  digitalWrite(latchPin, LOW);

  shiftOut(
    dataPin,
    clockPin,
    MSBFIRST,
    ledState
  );

  digitalWrite(latchPin, HIGH);

  // =====================================
  // TOTAL DEMAND
  // =====================================

  int totalLoad = 0;

  for (int i = 0; i < 8; i++) {
    totalLoad += load[i];
  }

  int solarUsed = solarPower - remainingSolar;

  int gridTotal = totalLoad - solarUsed;

  // =====================================
  // SERIAL MONITOR
  // =====================================

  Serial.println();
  Serial.println("==============================");

  Serial.print("Solar Available: ");
  Serial.print(solarPower);
  Serial.println(" W");

  Serial.print("Total Demand:    ");
  Serial.print(totalLoad);
  Serial.println(" W");

  Serial.print("Solar Used:      ");
  Serial.print(solarUsed);
  Serial.println(" W");

  Serial.print("Grid Required:   ");
  Serial.print(gridTotal);
  Serial.println(" W");

  Serial.println("------------------------------");

  for (int i = 0; i < 8; i++) {

    Serial.print("Flat ");
    Serial.print(i + 1);

    Serial.print(" | Load: ");
    Serial.print(load[i]);

    Serial.print(" W | Solar: ");
    Serial.print(solarGiven[i]);

    Serial.print(" W | Grid: ");
    Serial.print(gridPower[i]);

    Serial.println(" W");
  }

  // =====================================
  // LCD PAGE 1
  // =====================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SOL:");
  lcd.print(solarPower);
  lcd.print("W");

  lcd.setCursor(0, 1);
  lcd.print("LOAD:");
  lcd.print(totalLoad);
  lcd.print("W");

  delay(2000);

  // =====================================
  // LCD PAGE 2
  // =====================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("USED:");
  lcd.print(solarUsed);
  lcd.print("W");

  lcd.setCursor(0, 1);
  lcd.print("GRID:");
  lcd.print(gridTotal);
  lcd.print("W");

  delay(2000);

  // =====================================
  // LCD PAGE 3
  // =====================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("F1:");
  lcd.print(solarGiven[0]);

  lcd.setCursor(9, 0);
  lcd.print("F2:");
  lcd.print(solarGiven[1]);

  lcd.setCursor(0, 1);
  lcd.print("F3:");
  lcd.print(solarGiven[2]);

  lcd.setCursor(9, 1);
  lcd.print("F4:");
  lcd.print(solarGiven[3]);

  delay(2000);

  // =====================================
  // LCD PAGE 4
  // =====================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("F5:");
  lcd.print(solarGiven[4]);

  lcd.setCursor(9, 0);
  lcd.print("F6:");
  lcd.print(solarGiven[5]);

  lcd.setCursor(0, 1);
  lcd.print("F7:");
  lcd.print(solarGiven[6]);

  lcd.setCursor(9, 1);
  lcd.print("F8:");
  lcd.print(solarGiven[7]);

  delay(2000);
}
