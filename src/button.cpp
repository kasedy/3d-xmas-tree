#include "button.h"

void Button::loop() {
  uint32_t now = millis();
  bool isPressed = (digitalRead(pin) == LOW);
  if (isPressedLastTime && !isPressed) {
    // Button relesed
    if (now - lastDownTime > DOUBLE_CLICK_TIME_THRESHOLD) {
      rapidClickCounter = 0;
    } else {
      ++rapidClickCounter;
    }

    if (now - lastDownTime > CLICK_TIME_THRESHOLD) {
      // Finalize press event if it happened
      onPress(rapidClickCounter, PRESS_FINISH);
    }
  } else if (!isPressedLastTime && isPressed) {
    // Button pressed
    lastDownTime = now;
    startHandlingLongPress = true;
  }
  isPressedLastTime = isPressed;

  if (!isPressed && now - lastDownTime > DOUBLE_CLICK_TIME_THRESHOLD) {
    onClick(rapidClickCounter);
    rapidClickCounter = 0;
  }

  if (isPressed && now - lastDownTime > CLICK_TIME_THRESHOLD) {
    onPress(rapidClickCounter, startHandlingLongPress ? PRESS_START : PRESS_ONGOING);
    startHandlingLongPress = false;
  }
}  