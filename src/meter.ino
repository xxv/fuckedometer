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

#define UPDATE_DELAY_MIN 1
// Calibrate this manually. This was calibrated using a 56k resistor
#define METER_OFFSET  100
#define METER_MAX     ((METER_OFFSET * PWMRANGE) / 100)

#define HAPPY_MAX  2
#define FUCKED_MIN 98

const char *http_url = "http://fuckedometer.com/fuckedometer";

////////////////////////////

#define MINUTE_MS 60 * 1000

WiFiManager wifiManager;
Ticker ticker;
Ticker blink_ticker;
HTTPClient http;

bool statusLed = HIGH;

CRGB statusLeds[NUM_LEDS];

unsigned long percent = 0;

unsigned long httpToPercentageBare(HTTPClient *client) {
  return client->getString().toInt();
}

void blink_error(void) {
  for (uint8_t i = 0; i < 3; i++) {
    statusLeds[0] = CRGB::Red;
    FastLED.show();
    delay(250);
    statusLeds[0] = CRGB::Black;
    FastLED.show();
    delay(250);
  }
}

void update_meter(unsigned long percentage) {
  analogWrite(PIN_METER, map(percentage, 0, 100, 0, METER_MAX));
  percent = percentage;
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
  update_meter(50);

  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect("Fuckedometer")) {
    ESP.reset();
    delay(1000);
  }

  ticker.detach();

  statusLeds[0] = CRGB::Yellow;
  FastLED.show();
  blink_ticker.attach(0.1, blink_tick);
}

void tick() {
  statusLeds[0] = statusLed ? CRGB::White : CRGB::Black;
  FastLED.show();
  statusLed = !statusLed;
}

int blink_frame = 0;
const int frame_count = 30;
const int red_step = 255 / (frame_count/2);

void blink_tick() {
  if (percent <= HAPPY_MAX) {
    uint8_t segment = blink_frame / (frame_count/4);
    switch (segment) {
      case 0:
      statusLeds[0] = CRGB::Red;
      break;
      case 1:
      statusLeds[0] = CRGB::White;
      break;
      case 2:
      statusLeds[0] = CRGB::Blue;
      break;
      case 3:
      statusLeds[0] = CRGB::Black;
      break;
    }
    FastLED.show();
  } else if (percent >= FUCKED_MIN) {
    statusLeds[0].blue = 0;
    statusLeds[0].green = 0;
    statusLeds[0].red = blink_frame <= frame_count/2
      ? red_step * blink_frame
      : red_step * (frame_count/2 - blink_frame);
    FastLED.show();
  }

  blink_frame = (blink_frame + 1) % frame_count;
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
    update_meter(httpToPercentageBare(&http));
    statusLeds[0] = CRGB::Black;
    FastLED.show();
  } else {
    blink_error();
  }

  for (uint8_t i = 0; i < UPDATE_DELAY_MIN; i++) {
    delay(MINUTE_MS);
  }
}
