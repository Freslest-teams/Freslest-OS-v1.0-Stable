#include "apps.h"
#include "gfx.h"
#include "input.h"
#include "settings.h"
#include <WiFi.h>
#include <time.h>

static uint32_t lastDraw = 0;
static bool ntpStarted = false;

static void ensureNTP() {
  if (ntpStarted) return;
  // If WiFi is connected, try NTP.
  if (WiFi.status() == WL_CONNECTED) {
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    ntpStarted = true;
  }
}

static void drawClock() {
  tft.fillScreen(TH().bg);
  drawHeader("Apps: Clock");
  drawFooter("OK Refresh", "BACK Menu");
  clearBody();

  ensureNTP();

  tft.setTextColor(TH().text, TH().bg);
  tft.setTextSize(3);

  struct tm timeinfo;
  bool hasTime = getLocalTime(&timeinfo, 50);

  int x = 40, y = 105;
  if (hasTime) {
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
    tft.setCursor(x, y);
    tft.print(buf);

    tft.setTextSize(2);
    strftime(buf, sizeof(buf), "%d.%m.%Y", &timeinfo);
    tft.setCursor(x, y + 55);
    tft.print(buf);

    tft.setTextSize(1);
    tft.setCursor(x, y + 90);
    tft.setTextColor(TH().dim, TH().bg);
    tft.print("NTP time (needs WiFi connected).");
  } else {
    // fallback uptime clock
    uint32_t s = millis()/1000;
    uint32_t hh = (s/3600)%100;
    uint32_t mm = (s/60)%60;
    uint32_t ss = s%60;
    char buf[32];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", (unsigned long)hh, (unsigned long)mm, (unsigned long)ss);
    tft.setCursor(x, y);
    tft.print(buf);

    tft.setTextSize(1);
    tft.setTextColor(TH().dim, TH().bg);
    tft.setCursor(x, y + 60);
    tft.print("No NTP yet. Connect WiFi in Settings.");
  }

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(40, 240);
  tft.print("WiFi: ");
  tft.print(WiFi.status() == WL_CONNECTED ? "Connected" : "Not connected");
}

void clockEnter() {
  lastDraw = 0;
  drawClock();
}

void clockLoop(uint32_t now) {
  if (bOk.pressed() || now - lastDraw > 1000) {
    lastDraw = now;
    drawClock();
  }
}
