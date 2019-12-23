#pragma once
#include <Arduino.h>
#include <SendOnlySoftwareSerial.h>

extern SendOnlySoftwareSerial serial;

#define CLICK_TIME_THRESHOLD 500
#define DOUBLE_CLICK_TIME_THRESHOLD 250

class Button {
public:  
  enum PressType {
    PRESS_START,
    PRESS_ONGOING,
    PRESS_FINISH,
  };

  typedef void (*ClickHandler)(uint8_t numClicks);
  typedef void (*PressEvent)(uint8_t numClicks, PressType eventType);

private:
  const uint8_t pin;
  bool isPressedLastTime;
  bool startHandlingLongPress;
  uint32_t lastDownTime;
  uint8_t rapidClickCounter;
  ClickHandler clickHandler;
  PressEvent longPressHandler;
public:
  Button(uint8_t pin) : 
      pin(pin), 
      lastDownTime(0), 
      rapidClickCounter(0), 
      clickHandler(nullptr), 
      longPressHandler(nullptr) {
    pinMode(pin, INPUT_PULLUP);
  }

  void setClickHandler(ClickHandler handler) {
    clickHandler = handler;
  }

  void setLongPressHandler(PressEvent handler) {
    longPressHandler = handler;
  }

  void loop();
  
private:
    void onClick(uint8_t numClicks) {
    if (clickHandler) {
      clickHandler(numClicks);
    }
  }

  void onPress(uint8_t numClicks, PressType pressType) {
    if (longPressHandler) {
      longPressHandler(numClicks, pressType);
    }
  }
};