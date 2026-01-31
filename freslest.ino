/*
  FreslestOS ULTRA PERF v1.0 (Multi-file)
  ESP32-S3 + ILI9488 SPI + LovyanGFX (latest)
  - No full-screen sprite (fast)
  - Partial redraw everywhere (menu/test/game/task)
  - Task Switcher (hold OK)
  - Preferences: theme (dark/light). (Brightness UI only; BL is tied to 3V3)
  - Apps: Apps, Settings, System Info, Test, Game (Dino)

  Your wiring:
  MOSI=11 SCK=12 CS=10 DC=9 RST=8 MISO=13 BL=3V3
  Buttons: UP=4 DOWN=5 LEFT=6 RIGHT=7 OK=15 BACK=16
*/

#include "audio_manager.h"
#include <Arduino.h>
#include <LittleFS.h>

#include "gfx.h"
#include "input.h"
#include "settings.h"
#include "apps.h"
#include "audio_manager.h"
void setup() {
  audioInit();
  audioBoot();

  Serial.begin(115200);

  // FS for Note app
  LittleFS.begin(true);

  inputBegin();
  gfxBegin();       // tft.init + rotation
  settingsLoad();
  audioInit();

  randomSeed(esp_random());

  fpsInit();

  bootScreen();
  drawMenuFull();
}

void loop() {
  const uint32_t now = millis();

  // frame limiter
  if (!frameReady(now)) return;

  inputUpdate(now);
  audioUpdate(now);
  fpsTick(now);

  // OK hold => task switcher (global)
  taskSwitcherHandle(now);

  // app logic + draws (batched SPI)
  tft.startWrite();
  appLoop(now);
  tft.endWrite();
}
