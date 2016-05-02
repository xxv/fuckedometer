#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ESP8266HTTPClient.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

////////////////////////////
// Configurable stuff
////////////////////////////

#define PIN_METER 4
#define NUM_LEDS  1

#define UPDATE_DELAY_MIN 1

#define HAPPY_MAX  2
#define SAD_MIN    98

const char *http_url   = "http://fuckedometer.com/fuckedometer";
const char *config_url = "http://fuckedometer.com/c/";

////////////////////////////

uint pref_blink     = 255;      // the LED brightness when blinking for "on"
uint pref_meter_max = PWMRANGE; // the meter max value

////////////////////////////

#define MINUTE_MS 60 * 1000

enum DeviceMode {
  booting = 0,
  normal,
  wifi_setup
};

enum ErrorStatus {
    no_error = 0,
    http_client,
    http_server,
    wifi_disconnected
};

ErrorStatus error_status = no_error;
DeviceMode device_mode = booting;

WiFiManager wifiManager;
Ticker blink_ticker;
HTTPClient http;
char request_url[64];
char device_id[9];

CRGB statusLeds[NUM_LEDS];
uint8_t on_light = 0;
int blink_frame = 0;
const int frame_count = 30;
const int red_step = 255 / (frame_count/2);

unsigned long percent = 0;

uint8_t mac[WL_MAC_ADDR_LENGTH];

unsigned long httpToPercentageBare(HTTPClient *client) {
  return client->getString().toInt();
}

void update_meter(unsigned long percentage) {
  analogWrite(PIN_METER, map(percentage, 0, 100, 0, pref_meter_max));
  percent = percentage;
}

void blink_tick() {
  CRGB color;
  if (device_mode == normal) {
    if (error_status == no_error) {
      if (percent <= HAPPY_MAX) {
        uint8_t segment = blink_frame / (frame_count/4);
        switch (segment) {
          case 0:
          color = CRGB::Red;
          break;

          case 1:
          color = CRGB::White;
          break;

          case 2:
          color = CRGB::Blue;
          break;

          default:
          case 3:
          color = CRGB::Black;
          break;
        }
        FastLED.showColor(color);
      } else if (percent >= SAD_MIN) {
        color.blue = 0;
        color.green = 0;
        color.red = blink_frame <= frame_count/2
          ? red_step * blink_frame
          : red_step * (frame_count/2 - blink_frame);
        FastLED.showColor(color);
      } else {
        if (blink_frame == 0) {
          on_light = (on_light + 1) % 3;
        }

        FastLED.showColor((on_light == 0 && blink_frame == 0)
                          ? CRGB(pref_blink,pref_blink,pref_blink) : CRGB::Black);
      }
    } else {
      color = CRGB::Red;

      if (error_status == http_client) {
        color = CRGB::Orange;
      }

      FastLED.showColor((((blink_frame / 5) % 2) == 0) ? color : CRGB::Black);
    }
  } else if (device_mode == wifi_setup) {
    FastLED.showColor((((blink_frame / 5) % 2) == 0) ? CRGB::Yellow : CRGB::Black);
  }

  blink_frame = (blink_frame + 1) % frame_count;
}

void configModeCallback(WiFiManager *myWiFiManager) {
  device_mode = wifi_setup;
}

void loadConfigFromServer(const char *device) {
  StaticJsonBuffer<200> jsonBuffer;
  char url[64];

  sprintf(url, "%s%s", config_url, device_id);
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode == 200) {
    error_status = no_error;
    JsonObject& config = jsonBuffer.parseObject(http.getString());
    if (config["reset"]) {
      FastLED.showColor(CRGB::DarkViolet);
      wifiManager.resetSettings();
      delay(100);
      ESP.reset();
    }

    pref_blink = config["blink"];

    pref_meter_max = config["meter_max"];

    if (pref_meter_max > PWMRANGE) {
      pref_meter_max = PWMRANGE;
    }

    if (pref_meter_max) {
      save_max(pref_meter_max);
    }
  }
  http.end();
}

uint load_max() {
  return EEPROM.read(0) | (EEPROM.read(1) << 8);
}

void save_max(uint max) {
  EEPROM.write(0, max & 0xff);
  EEPROM.write(1, (max >> 8) & 0xff);
  EEPROM.commit();
}

void setup() {
  EEPROM.begin(128);
  pref_meter_max = load_max();

  if (pref_meter_max == 0 || pref_meter_max > PWMRANGE) {
    pref_meter_max = PWMRANGE;
  }

  FastLED.addLeds<APA102, MOSI, SCK, BGR>(statusLeds, NUM_LEDS);
  FastLED.showColor(CRGB::Black);

  pinMode(PIN_METER, OUTPUT);
  analogWrite(PIN_METER, 0);
  // Gently sweep the meter
  for (int i = 0; i < pref_meter_max; i++) {
    analogWrite(PIN_METER, i);
    delay(1);
    FastLED.showColor(CHSV(0,0,map(i, 0, pref_meter_max, 0, 255)));
  }

  blink_ticker.attach(0.1, blink_tick);

  delay(500);
  // Put the needle at the center
  update_meter(50);

  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect("Fuckedometer")) {
    ESP.reset();
    delay(1000);
  }

  device_mode = normal;

  WiFi.macAddress(mac);
  sprintf(device_id, "%02x%02x%02x%02x", mac[2], mac[3], mac[4], mac[5]);
  sprintf(request_url, "%s?id=%s", http_url, device_id);

  FastLED.showColor(CRGB::Yellow);

  loadConfigFromServer(device_id);
}

void loop() {
  http.begin(request_url);

  int httpCode = http.GET();
  if(httpCode == 200) {
    error_status = no_error;
    update_meter(httpToPercentageBare(&http));
  } else if (httpCode >= 400 && httpCode < 500) {
    error_status = http_client;
  } else if (httpCode >= 500 && httpCode < 600) {
    error_status = http_server;
  } else {
    error_status = http_client;
  }
  http.end();

  for (uint8_t i = 0; i < UPDATE_DELAY_MIN; i++) {
    delay(MINUTE_MS);
  }
}
