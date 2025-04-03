#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <PZEM004Tv30.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "EmonLib.h"  // Include Emon Library
#include <ZMPT101B.h>

#define SENSITIVITY 498.25f

EnergyMonitor emon1;  // Create an instance
EnergyMonitor emon2;  // Create an instance
EnergyMonitor emon3;  // Create an instance
// EnergyMonitor emon;   // Create an instance
ZMPT101B voltageSensor(35, 50.0);

#define AmpSensor1 33
#define AmpSensor2 39  //VN
#define AmpSensor3 34
// #define MainAmpSensor 36  //VP
#define SSR1 25
#define SSR2 26
#define SSR3 27
#define Button1 23
#define Button2 19
#define Button3 18
#define dcvoltagepin 32

// #define RXD2 16  // to TX of Pzem module
// #define TXD2 17  // to RX of Pzem module
// PZEM004Tv30 pzem(Serial2, RXD2, TXD2);
LiquidCrystal_I2C lcd(0x3F, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

String Mode = "0";        // 0 -> Manual, 1 -> Auto
String unitsReset = "0";  // 0 -> Not Reset, 1 -> Reset units
String btn1 = "0", btn2 = "0", btn3 = "0";
String load1 = "OFF", load2 = "OFF", load3 = "OFF";

// Parameters
float PowerT = 0.0, Power1 = 0.0, Power2 = 0.0, Power3 = 0.0;
float AmpsT = 0.0, Amps1 = 0.0, Amps2 = 0.0, Amps3 = 0.0;
float voltage = 0.0, pf = 0.0, freq = 0.0, units = 0.0;
String unitsSet = "0";
unsigned long lastMillis = 0, previousMillis = 0, unitsMillis = 0;
bool swapScreen = false;
int ACS712refreshCount = 0;

// Floats for resistor values in voltage divider (for DC voltage)
float R1 = 10000.0;
float R2 = 2200.0;
float ref_voltage = 3.3;
// Integer for ADC value
int adc_value = 0;

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Hashir"
#define WIFI_PASSWORD "12345678"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBsX8j8yb7uYshGlMBJVUvd0Rl5JJM-i9o"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://smartgrid-abc67-default-rtdb.firebaseio.com/"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

uint32_t sendDataPrevMillis = 0;
uint32_t readDataPrevMillis = 0;
bool signupOK = false;
volatile bool dataChanged = false;

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  lcd.setCursor(0, 1);
  lcd.print("Internet Connected.");

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  lcd.setCursor(0, 2);
  lcd.print("Firebase Connected");
}

void initPins() {
  voltageSensor.setSensitivity(SENSITIVITY);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Power Management of");
  lcd.setCursor(0, 1);
  lcd.print("Various Sources");

  pinMode(Button1, INPUT_PULLUP);
  pinMode(Button2, INPUT_PULLUP);
  pinMode(Button3, INPUT_PULLUP);
  emon1.current(AmpSensor1, 2.9);
  emon2.current(AmpSensor2, 2.9);
  emon3.current(AmpSensor3, 2.9);
  // emon.current(MainAmpSensor, 0.8);  // Current: input pin, calibration.

  pinMode(SSR1, OUTPUT);
  digitalWrite(SSR1, LOW);
  pinMode(SSR2, OUTPUT);
  digitalWrite(SSR2, LOW);
  pinMode(SSR3, OUTPUT);
  digitalWrite(SSR3, LOW);

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

float scanDCvolts() {
  float Vin = 0.0, Vout = 0.0;

  // Read the Analog Input
  adc_value = analogRead(dcvoltagepin);
  Vout = (adc_value * ref_voltage) / 4095.0;

  // Calculate voltage at divider input
  Vin = ((Vout * (R1 + R2) / R2)) + 1.2;
  return Vin;
}

void sentdata() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3500 || sendDataPrevMillis == 0)) {
    Serial.println("Data Sent");

    // Writing values to Firebase Realtime database
    Firebase.RTDB.setString(&fbdo, "/Data/load1state", load1);
    Firebase.RTDB.setString(&fbdo, "/Data/load2state", load2);
    Firebase.RTDB.setString(&fbdo, "/Data/load3state", load3);
    Firebase.RTDB.setString(&fbdo, "/Data/power", String(PowerT,1));
    Firebase.RTDB.setString(&fbdo, "/Data/current", String(AmpsT,2));
    Firebase.RTDB.setString(&fbdo, "/Data/pow1", String(Power1,1));
    Firebase.RTDB.setString(&fbdo, "/Data/pow2", String(Power2,1));
    Firebase.RTDB.setString(&fbdo, "/Data/pow3", String(Power3,1));
    Firebase.RTDB.setString(&fbdo, "/Data/amps1", String(Amps1,2));
    Firebase.RTDB.setString(&fbdo, "/Data/amps2", String(Amps2,2));
    Firebase.RTDB.setString(&fbdo, "/Data/amps3", String(Amps3,2));
    Firebase.RTDB.setString(&fbdo, "/Data/units", String(units,3));
    // Firebase.RTDB.setFloat(&fbdo, "/Data/pf", pf);
    // Firebase.RTDB.setFloat(&fbdo, "/Data/freq", freq);
    Firebase.RTDB.setString(&fbdo, "/Data/voltage", String(voltage,1));
    Firebase.RTDB.setString(&fbdo, "/Data/syncVolt", String(scanDCvolts(),1));
    sendDataPrevMillis = millis();
  }
}

void read_data() {
  if (Firebase.ready() && signupOK && (millis() - readDataPrevMillis > 3000)) {
    digitalWrite(2, HIGH);

    if (Firebase.RTDB.getString(&fbdo, "/Buttons/mode")) {
      if (fbdo.dataType() == "string") {
        Mode = fbdo.stringData();
        // Serial.println("Mode => " + Mode);
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/Buttons/btn1")) {
      if (fbdo.dataType() == "string") {
        btn1 = fbdo.stringData();
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/Buttons/btn2")) {
      if (fbdo.dataType() == "string") {
        btn2 = fbdo.stringData();
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/Buttons/btn3")) {
      if (fbdo.dataType() == "string") {
        btn3 = fbdo.stringData();
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/Buttons/unitsSet")) {
      if (fbdo.dataType() == "string") {
        unitsSet = fbdo.stringData();
        // Serial.println("Units Set=> " + unitsSet);
      }
    }
    if (Firebase.RTDB.getString(&fbdo, "/Buttons/unitsReset")) {
      if (fbdo.dataType() == "string") {
        unitsReset = fbdo.stringData();
        // Serial.println("Units Reset => " + unitsReset);
        if (unitsReset == "1") {
          units = 0;
          delay(1000);
          Firebase.RTDB.setString(&fbdo, "/Buttons/unitsReset", "0");
        }
      }
    }
    digitalWrite(2, LOW);
    readDataPrevMillis = millis();
  }
}



void scanButtons() {
  if (digitalRead(Button1) == 0 || btn1 == "ON") {
    delay(100);
    if (digitalRead(Button1) == 0 || btn1 == "ON") {
      load3 = "ON";
      digitalWrite(SSR1, HIGH);
    }
  } else {
    load3 = "OFF";
    digitalWrite(SSR1, LOW);
  }
  if (digitalRead(Button2) == 0 || btn2 == "ON") {
    delay(100);
    if (digitalRead(Button2) == 0 || btn2 == "ON") {
      load2 = "ON";
      digitalWrite(SSR2, HIGH);
    }
  } else {
    load2 = "OFF";
    digitalWrite(SSR2, LOW);
  }
  if (digitalRead(Button3) == 0 || btn3 == "ON") {
    delay(100);
    if (digitalRead(Button3) == 0 || btn3 == "ON") {
      load1 = "ON";
      digitalWrite(SSR3, HIGH);
    }
  } else {
    load1 = "OFF";
    digitalWrite(SSR3, LOW);
  }
}

// bool PzemDisplay() {
//   bool success = true;

//   // Read the data from the sensor
//   voltage = pzem.voltage();
//   AmpsT = pzem.current();
//   PowerT = pzem.power();
//   units = pzem.energy();
//   freq = pzem.frequency();
//   pf = pzem.pf();

//   // Check if the data is valid
//   if (isnan(voltage)) {
//     voltage = 0;
//     success = false;
//   }
//   return success;
// }



void lcdUpdate2() {
  lcd.setCursor(0, 0);
  lcd.print("Voltage = ");
  lcd.print(voltage, 1);
  lcd.print(" V");
  lcd.setCursor(0, 1);
  lcd.print("Current = ");
  lcd.print(AmpsT, 2);
  lcd.print(" A");
  lcd.setCursor(0, 2);
  lcd.print("Power = ");
  lcd.print(PowerT, 2);
  lcd.print(" W");
  lcd.setCursor(0, 3);
  lcd.print("Units = ");
  lcd.print(units, 2);
  lcd.print(" KWh");
}

void lcdUpdate1() {
  lcd.setCursor(0, 0);
  lcd.print("I= ");
  lcd.print(Amps1, 2);
  lcd.print("A  ");
  lcd.print("P=");
  lcd.print(Power1, 1);
  lcd.print("W");

  lcd.setCursor(0, 1);
  lcd.print("I= ");
  lcd.print(Amps2, 2);
  lcd.print("A  ");
  lcd.print("P=");
  lcd.print(Power2, 1);
  lcd.print("W");

  lcd.setCursor(0, 2);
  lcd.print("I= ");
  lcd.print(Amps3, 2);
  lcd.print("A  ");
  lcd.print("P=");
  lcd.print(Power3, 1);
  lcd.print("W");
}
