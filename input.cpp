#include "input.h"
#include "audio_manager.h"

Button bUp, bDown, bLeft, bRight, bOk, bBack;

void Button::begin(int p) {
  pin = p;
  pinMode(pin, INPUT_PULLUP);
  stable = lastRead = digitalRead(pin);
  lastChange = millis();
}

void Button::update(uint32_t now, uint16_t db) {
  pressedEdge = false;
  bool r = digitalRead(pin);
  if (r != lastRead) { lastRead = r; lastChange = now; }
  if ((now - lastChange) >= db && stable != lastRead) {
    stable = lastRead;
    if (stable == LOW) pressedEdge = true;
  }
}

void inputBegin() {
  bUp.begin(BTN_UP);
  bDown.begin(BTN_DOWN);
  bLeft.begin(BTN_LEFT);
  bRight.begin(BTN_RIGHT);
  bOk.begin(BTN_OK);
  bBack.begin(BTN_BACK);
}

void inputUpdate(uint32_t now) {
  bUp.update(now);
  bDown.update(now);
  bLeft.update(now);
  bRight.update(now);
  bOk.update(now);
  bBack.update(now);
  if (bUp.pressed()||bDown.pressed()||bLeft.pressed()||bRight.pressed()||bOk.pressed()||bBack.pressed()) {
    audioClick();
  }
}
