#include "Secrets.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define PIN_STATUS_LED 5

// every 10 minutes
#define UPDATE_DELAY_MS 10 * 60 * 1000

ESP8266WiFiMulti wifi;

bool statusLed = HIGH;

int httpToPercentage(HTTPClient client) {
  const char *response = client.getString().c_str();
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject((char *)response);

  JsonArray& latest = root["latest"];
  JsonArray& dem = latest[0];
  const char *demPercent = dem[1];

  return strtol(demPercent, 0, 10);
}

void setup() {
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(13, INPUT);

  Serial.begin(115200);

  Serial.println("Connecting...");
  wifi.addAP(ap_name, ap_password);
}

void connected_loop() {
  HTTPClient http;

  http.begin(http_url);

  int httpCode = http.GET();
  if(httpCode) {
      Serial.printf("Success!: %d\n", httpCode);
      int percent = httpToPercentage(http);
      Serial.printf("Percentage is %d\n", percent);
      for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(PIN_STATUS_LED, HIGH);
        delay(100);
        digitalWrite(PIN_STATUS_LED, LOW);
        delay(100);
      }
      delay(1000);

      for (uint8_t i = 0; i < percent / 10; i++) {
        digitalWrite(PIN_STATUS_LED, HIGH);
        delay(100);
        digitalWrite(PIN_STATUS_LED, LOW);
        delay(100);
      }
  } else {
      Serial.write("Error GETing\n");
  }

  delay(UPDATE_DELAY_MS);
}

void loop() {
  digitalWrite(PIN_STATUS_LED, statusLed);
  // wait for WiFi connection
   if((wifi.run() == WL_CONNECTED)) {
      for (uint8_t i = 0; i < 3; i++) {
      digitalWrite(PIN_STATUS_LED, HIGH);
      delay(50);
      digitalWrite(PIN_STATUS_LED, LOW);
      delay(50);
    }
        Serial.print("Connected!\n");

        while (wifi.run() == WL_CONNECTED) {
          connected_loop();
        }

        // Turn off again
        digitalWrite(PIN_STATUS_LED, LOW);
    }

  delay(500);

  statusLed = statusLed == LOW ? HIGH : LOW;
}
