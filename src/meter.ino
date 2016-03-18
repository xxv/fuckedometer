#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ESP8266HTTPClient.h>
#include <FastLED.h>
#include <Ticker.h>

////////////////////////////
// Configurable stuff
////////////////////////////

#define PIN_METER 4
#define NUM_LEDS  1

// every 10 minutes
#define UPDATE_DELAY_MIN 10
// Calibrate this manually. This was calibrated using a 56k resistor
#define METER_MAX 900

const char *http_url = "http://xxv.so/fuckedometer";

////////////////////////////

#define MINUTE_MS 60 * 1000

WiFiManager wifiManager;
Ticker ticker;
HTTPClient http;

bool statusLed = HIGH;

CRGB statusLeds[NUM_LEDS];

unsigned long httpToPercentageBare(HTTPClient *client) {
  return client->getString().toInt();
}

void setup() {
  FastLED.addLeds<APA102, MOSI, SCK, BGR>(statusLeds, NUM_LEDS);
  statusLeds[0] = CRGB::Black;
  FastLED.show();

  pinMode(PIN_METER, OUTPUT);
  analogWrite(PIN_METER, 0);
  // Gently sweep the meter
  for (int i = 0; i < METER_MAX; i++) {
    analogWrite(PIN_METER, i);
    delay(1);
  }

  delay(1000);
  // Put the needle at the center
  analogWrite(PIN_METER, METER_MAX/2);

  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect("Fuckedometer")) {
    ESP.reset();
    delay(1000);
  }

  ticker.detach();

  statusLeds[0] = CRGB::Yellow;
  FastLED.show();
}

void tick() {
  statusLeds[0] = statusLed ? CRGB::White : CRGB::Black;
  FastLED.show();
  statusLed = !statusLed;
}

void configModeCallback (WiFiManager *myWiFiManager) {
  ticker.attach(0.2, tick);
}

void loop() {
  http.begin(http_url);

  int httpCode = http.GET();
  if(httpCode == 200) {
    statusLeds[0] = CRGB::Green;
    FastLED.show();
    unsigned long percent = httpToPercentageBare(&http);
    statusLeds[0] = CRGB::Black;
    FastLED.show();

    analogWrite(PIN_METER, map(percent, 0, 100, 0, METER_MAX));
  } else {
    statusLeds[0] = CRGB::Red;
    FastLED.show();
    delay(500);
    statusLeds[0] = CRGB::Black;
    FastLED.show();
  }

  for (uint8_t i = 0; i < UPDATE_DELAY_MIN; i++) {
    delay(MINUTE_MS);
  }
}
