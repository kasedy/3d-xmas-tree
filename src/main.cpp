#include <Arduino.h>
#include <FastLED.h>
#include <SendOnlySoftwareSerial.h>

#include "button.h"

#define LED_PIN     3
#define NUM_LEDS    12
#define START_BRIGHTNESS  60
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

#define UPDATES_PER_SECOND 100

#define BUTTON_PIN 4

SendOnlySoftwareSerial serial(2);

CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

Button button(BUTTON_PIN);

uint32_t lastAnimationTime = 0;
uint8_t ledStartIndex = 0;
bool lockAnimation = false;

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(currentPalette, colorIndex + (255 / NUM_LEDS * i), START_BRIGHTNESS, currentBlending);
  }
}

void longPressHandler(uint8_t numClicks, Button::PressType eventType) {
  static uint32_t lastChangeBrightness = 0;
  static bool lightScrollDirectionUp = true;

  if (numClicks == 0) {
    if (eventType == Button::PRESS_START) {
      lightScrollDirectionUp = !lightScrollDirectionUp;
      lockAnimation = true;
    } else if (eventType == Button::PRESS_FINISH) {
      lockAnimation = false;
    } else if (eventType == Button::PRESS_ONGOING) {
      uint32_t now = millis();
      if (now - lastChangeBrightness >= 10) {
        lastChangeBrightness = now;
        uint8_t brightness = FastLED.getBrightness();
        uint8_t step = 1;
        if (brightness > 255 - step) {
          lightScrollDirectionUp = false;
        } else if (brightness < step + 1) { // Min Brightness = 1
          lightScrollDirectionUp = true;
        }
        FastLED.setBrightness(brightness + (lightScrollDirectionUp ? step : -step));
        FastLED.show();
      }
    }
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(START_BRIGHTNESS);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  button.setLongPressHandler(longPressHandler);

  serial.begin(115200);
}

void loop()
{
  uint32_t now = millis();
  if (now - lastAnimationTime >= 1000 / UPDATES_PER_SECOND && !lockAnimation) {
    ledStartIndex += 1;
    FillLEDsFromPaletteColors(ledStartIndex);
    FastLED.show();
    lastAnimationTime = now;
  }

  button.loop();
}