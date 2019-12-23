#pragma once

#include <FastLED.h>

class Animation {
  CRGB *leds;
  uint8_t numLeds;
  CRGBPalette16 palette;
  TBlendType blending;
  uint8_t colorIndex;

public:
  Animation(CRGB leds[], uint8_t numLeds, CRGBPalette16 palette, TBlendType blending) :
      leds(leds), numLeds(numLeds), palette(palette), blending(blending), colorIndex(0) {}

  void loop() {
    for (int i = 0; i < numLeds; ++i) {
      leds[i] = ColorFromPalette(palette, colorIndex + (255 / numLeds * i), 255, blending);
    }
    colorIndex += 1;
    FastLED.show();
  }
};