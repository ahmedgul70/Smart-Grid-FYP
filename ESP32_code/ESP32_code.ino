#include "Config.h"

long interval = 60000;
void setup() {
  Serial.begin(115200);
  initPins();
  initWiFi();
  read_data();
  Serial.println("OK");
  for (int i = 0; i <= 6; i++) {
    Amps1 = emon3.calcIrms(2000);  // Calculate Irms only
    Amps2 = emon2.calcIrms(2000);  // Calculate Irms only
    Amps3 = emon1.calcIrms(2000);  // Calculate Irms only
    delay(1000);
  }
}

void loop() {
  if (Mode == "0") {
    scanButtons();
  } else if (Mode == "1") {  // Auto Mode
    if (units >= unitsSet.toFloat()) {
      LoadAuto(0);
    } else {
      LoadAuto(1);
    }
  }

  if (millis() - lastMillis >= 2000) {
    scanAmps();
    Lcd_Update();
    lastMillis = millis();
  }

  read_data();
  sentdata();

  // if WiFi is down, try reconnecting
  // if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillis >= 10000)) {
  //   Serial.println("Reconnecting to WiFi......");
  //   WiFi.disconnect();
  //   WiFi.reconnect();
  //   Serial.println("Reconnected");
  //   previousMillis = millis();
  // }
}

void LoadAuto(int i) {
  if (i == 1) {
    load1 = "ON";
    load2 = "ON";
    load3 = "ON";
    digitalWrite(SSR1, HIGH);
    digitalWrite(SSR2, HIGH);
    digitalWrite(SSR3, HIGH);
  } else {
    load1 = "OFF";
    load2 = "OFF";
    load3 = "OFF";
    digitalWrite(SSR1, LOW);
    digitalWrite(SSR2, LOW);
    digitalWrite(SSR3, LOW);
  }
}

void scanAmps() {
  voltage = voltageSensor.getRmsVoltage();
  Serial.println(voltage);
  // AmpsT = emon.calcIrms(2000);
  Amps1 = emon3.calcIrms(2000);  // Calculate Irms only
  Amps2 = emon2.calcIrms(2000);  // Calculate Irms only
  Amps3 = emon1.calcIrms(2000);  // Calculate Irms only

  if (Amps1 < 0.2) {
    Amps1 = 0.0;
  }
  if (Amps2 < 0.2) {
    Amps2 = 0.0;
  }
  if (Amps3 < 0.2) {
    Amps3 = 0.0;
  }
  AmpsT = Amps1 + Amps2 + Amps3;
  if (AmpsT < 0.2) {
    AmpsT = 0.0;
  }

  Power1 = voltage * Amps1;
  Power2 = voltage * Amps2;
  Power3 = voltage * Amps3;
  PowerT = Power1 + Power2 + Power3;

  if (millis() - unitsMillis > interval) {  // 1min reading
    units += (PowerT / 1000.0);
    Serial.print("Units : ");
    Serial.println(units);
    unitsMillis = millis();
  }
}

void debug() {
  Serial.print(voltage);
  Serial.print("\t");
  Serial.print(Amps1);
  Serial.print("\t");
  Serial.print(Amps2);
  Serial.print("\t");
  Serial.print(Amps3);
  Serial.print("\t");
  Serial.print(AmpsT);
  Serial.print("\t");
  Serial.println(scanDCvolts());
}

void Lcd_Update() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  if (swapScreen) {
    lcdUpdate2();
    swapScreen = false;
    Serial.println("LCD 1");
    ACS712refreshCount++;
  } else {
    // if (ACS712refreshCount >= 5) {  // due to initializing time of ACS712 sensors
      lcdUpdate1();
      Serial.println("LCD 2");
    // }
    swapScreen = true;
  }
}