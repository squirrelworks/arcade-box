#include <Mouse.h>

const int buttonPin = 2;      // arcade button on pin 2
bool lastButtonState = HIGH;   // previous reading (HIGH = not pressed)
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 30; // 30ms debounce — arcade buttons can be noisy
bool stableState = HIGH;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // enable internal pull-up resistor
  Mouse.begin();                      // initialize the Mouse library
}

void loop() {
  bool reading = digitalRead(buttonPin);

  // If the reading changed, reset the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If enough time has passed, accept the new state
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stableState) {
      stableState = reading;

      // Only click on the press (LOW), not the release
      if (stableState == LOW) {
        Mouse.click(MOUSE_LEFT);  // sends a press + release
      }
    }
  }

  lastButtonState = reading;
}
