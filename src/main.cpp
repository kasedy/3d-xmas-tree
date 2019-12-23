#include <Arduino.h>
#include <FastLED.h>
#include <SendOnlySoftwareSerial.h>

#include "button.h"
#include "animation.h"
#include "helpers.h"

#define LED_PIN     3
#define NUM_LEDS    12
#define START_BRIGHTNESS  60
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define COLOR_UPDATE_INTERVAL_MS 50
#define ANIMATOIN_SPEED_SCALE_PERCENTAGE 5 // 5% per a unit

#define BUTTON_PIN 4

SendOnlySoftwareSerial debug(2);

CRGB leds[NUM_LEDS];

Button button(BUTTON_PIN);

bool lockAnimation = false;

Animation *currentAnimation;

typedef Animation* (*AnimationConstructor)();

AnimationConstructor animations[] = {
  [] { return new Animation(leds, NUM_LEDS, RainbowColors_p, LINEARBLEND); },
  [] { return new Animation(leds, NUM_LEDS, RainbowStripeColors_p, NOBLEND); },
  [] { return new Animation(leds, NUM_LEDS, RainbowStripeColors_p, LINEARBLEND); },
  [] { return new Animation(leds, NUM_LEDS, CloudColors_p, LINEARBLEND); },
  [] { return new Animation(leds, NUM_LEDS, PartyColors_p, LINEARBLEND); },
};

uint8_t currentAnimationIndex = 0;
int8_t speedMultiplier = 0;

void longPressHandler(uint8_t numClicks, Button::PressType eventType) {
  static uint32_t lastTimeUpdate = 0;
  static bool lightScrollDirectionUp = true;

  if (numClicks == 0) {
    if (eventType == Button::PRESS_START) {
      lightScrollDirectionUp = !lightScrollDirectionUp;
      lockAnimation = true;
    } else if (eventType == Button::PRESS_FINISH) {
      lockAnimation = false;
      lastTimeUpdate = 0;
    } else if (eventType == Button::PRESS_ONGOING) {
      uint32_t now = millis();
      if (now - lastTimeUpdate >= 10) {
        lastTimeUpdate = now;
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

  static bool speedScrollDirectionUp = true;
  if (numClicks == 1) {
    if (eventType == Button::PRESS_START) {
      speedScrollDirectionUp = !speedScrollDirectionUp;
    } else if (eventType == Button::PRESS_FINISH) {
      lastTimeUpdate = 0;
    } else if (eventType == Button::PRESS_ONGOING) {
      uint32_t now = millis();
      // We will scroll from the fastest animation to slowest in 5 sec
      if (now - lastTimeUpdate >= 25) {
        lastTimeUpdate = now;
        uint8_t step = 1;
        if (speedMultiplier > 100 - step) {
          speedScrollDirectionUp = false;
        } else if (speedMultiplier < step - 100) { // Min Brightness = 1
          speedScrollDirectionUp = true;
        }
        speedMultiplier += (speedScrollDirectionUp ? step : -step);
      }
    }
  }
}

void clickHandler(uint8_t numClicks) {
  if (numClicks == 1) {
    currentAnimationIndex += 1;
    if (currentAnimationIndex >= ARRAY_LEN(animations)) {
      currentAnimationIndex = 0;
    }
    delete currentAnimation;
    currentAnimation = animations[currentAnimationIndex]();
  }
}

void setup() {
  debug.begin(115200);
  button.setLongPressHandler(longPressHandler);
  button.setClickHandler(clickHandler);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(START_BRIGHTNESS);

  currentAnimation = animations[currentAnimationIndex]();
}

uint32_t linearDelayScalerUs(uint32_t defaultUpdatePeriodMs, double scaler, double percentage) {
  if (scaler < 0) {
    return defaultUpdatePeriodMs * (1.0 + (-scaler) * percentage / 100.0) * 1000.0;
  }
  if (scaler > 0) {
    return defaultUpdatePeriodMs / (1.0 + scaler * percentage / 100.0) * 1000.0;
  }
  return defaultUpdatePeriodMs;
}

void loop()
{
  static uint32_t lastAnimationTime = 0;
  uint32_t now = micros();
  uint32_t updateInterval = 
      linearDelayScalerUs(COLOR_UPDATE_INTERVAL_MS, speedMultiplier, ANIMATOIN_SPEED_SCALE_PERCENTAGE);
  if (now - lastAnimationTime >= updateInterval && !lockAnimation) {
    currentAnimation->loop();
    lastAnimationTime = now;
  }

  button.loop();
}