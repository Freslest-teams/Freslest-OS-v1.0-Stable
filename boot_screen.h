
#pragma once
#include <LovyanGFX.hpp>

extern LGFX lcd;

inline void showBootScreen() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_CYAN, TFT_BLACK);
  lcd.setTextSize(2);

  lcd.setCursor(60, 120);
  lcd.print("FRESLEST OS");

  lcd.setTextSize(1);
  lcd.setCursor(90, 150);
  lcd.print("Booting...");

  for (int i = 0; i <= 255; i += 8) {
    lcd.setBrightness(i);
    delay(10);
  }

  delay(400);
}
