#include "Nicla_System.h"

bool blink = true;
bool lastButtonState = HIGH;

void setup() {
  nicla::begin();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON1, INPUT_PULLUP);  // bouton intégré
}

void loop() {
  bool buttonState = digitalRead(BUTTON1);

  // détecte un appui (transition HIGH → LOW)
  if (lastButtonState == HIGH && buttonState == LOW) {
    blink = !blink;  // change le mode
  }
  lastButtonState = buttonState;

  if (blink) {
    // Turn the built-in LED off
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);

  } else {
    // Turn the built-in LED on
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}
