#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define BLYNK_PRINT Serial
// ********* BLYNK *********
#define BLYNK_TEMPLATE_ID "TMPL3KVYYtmph"
#define BLYNK_TEMPLATE_NAME "smart irrigation"
#define BLYNK_AUTH_TOKEN "6z_CbfYMGnPEs1DmejAZJCZY5lSeZ2Ob"

#include <BlynkSimpleEsp32.h>
// ****************************************************

#define DHTPIN 5
#define DHTTYPE DHT11
#define MOISTURE_PIN 34
#define WATER_LEVEL_PIN 35

// ********* RELAY *********
#define RELAY_PIN 25
// ****************************************************

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* auth ="6z_CbfYMGnPEs1DmejAZJCZY5lSeZ2Ob";
const char* ssid = "Kruthika";
const char* password = "kruthikamgowda";

String serverURL = "https://aislyntech.com/Api/z-update.php?";
// Example: https://aislyntech.com/Api/z-update.php?moisture=10&temperature=20&humidity=30&waterlevel=40&id=1


void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();

  // ********* RELAY SETUP *********
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Relay OFF (active LOW)
  // ****************************************************

  // ********* BLYNK START *********
  Blynk.begin(auth, ssid, password);
  // ****************************************************

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print(".");
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(1500);
  lcd.clear();
}



void loop() {
  Blynk.run();

  // Read sensors
  int temperature = dht.readTemperature();
  int humidity = dht.readHumidity();
  int moistureValue = analogRead(MOISTURE_PIN);
  int waterValue = analogRead(WATER_LEVEL_PIN);

  // Convert analog values to %
  int moisturePercent = map(moistureValue, 4095, 0, 0, 370);
  int waterPercent = map(waterValue, 0, 4095, 0, 100);

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C H:");
  lcd.print(humidity, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("M:");
  lcd.print(moisturePercent);
  lcd.print("% WL:");
  lcd.print(waterPercent);
  lcd.print("%");

  // Debug output
  Serial.print("Temperature: "); Serial.print(temperature);
  Serial.print(" °C, Humidity: "); Serial.print(humidity);
  Serial.print(" %, Moisture: "); Serial.print(moisturePercent);
  Serial.print(" %, Water Level: "); Serial.print(waterPercent);
  Serial.println(" %");


  // ************ SIMPLE RELAY CONTROL + LOG EVENTS ************
  if ((moisturePercent < 40 && moisturePercent != 0) || (waterPercent < 40 && waterPercent != 0)) {

    digitalWrite(RELAY_PIN, LOW);   // Pump ON

 
    Blynk.logEvent("alert", "Pump Turned ON");

    Serial.println("PUMP ON");
  }
  else {

    digitalWrite(RELAY_PIN, HIGH);  // Pump OFF


    Blynk.logEvent("alert", "Pump Turned OFF");

    Serial.println("PUMP OFF");
  }
  // ***********************************************************


  // ************ SEND SENSOR DATA TO BLYNK ************
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, moisturePercent);
  Blynk.virtualWrite(V3, waterPercent);
  // ****************************************************



  // ************ SEND TO SERVER ************
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = serverURL +
                 "moisture=" + String(moisturePercent) +
                 "&temperature=" + String(temperature) +
                 "&humidity=" + String(humidity) +
                 "&waterlevel=" + String(waterPercent) +
                 "&id=1";

    Serial.println("Request URL: " + url);

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.print("Server Response: ");
      Serial.println(response);
    } else {
      Serial.print("Error sending request: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } 
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Disconnected");
    Serial.println("WiFi not connected");
  }


  delay(1000);
}
