#include "apps.h"
#include "gfx.h"
#include "input.h"
#include "settings.h"
#include "audio_manager.h"
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>

#include <BLEDevice.h>

// ===== Settings menu =====
static const char* SET_ITEMS[] = { "Brightness", "Theme", "WiFi", "Bluetooth", "Sound", "Flash Reset" };
static constexpr int SET_COUNT = sizeof(SET_ITEMS)/sizeof(SET_ITEMS[0]);
static int setIndex = 0;

static void drawSettingsRow(int i, bool selected) {
  int y = 70 + i * 38;
  tft.fillRect(18, y - 2, 444, 34, TH().bg);

  if (selected) {
    tft.fillRoundRect(20, y, 440, 30, 8, TH().selbg);
    tft.drawRoundRect(20, y, 440, 30, 8, TH().accent);
    tft.setTextColor(TH().text, TH().selbg);
    tft.setCursor(40, y + 8);
    tft.setTextSize(2);
    tft.print("> ");
    tft.print(SET_ITEMS[i]);
  } else {
    tft.setTextColor(TH().dim, TH().bg);
    tft.setCursor(40, y + 8);
    tft.setTextSize(2);
    tft.print("  ");
    tft.print(SET_ITEMS[i]);
  }
}

void drawSettingsMenuFull() {
  tft.fillScreen(TH().bg);
  drawHeader("Settings");
  drawFooter("UP/DOWN Move  OK Enter", "BACK Menu");
  clearBody();
  for (int i=0;i<SET_COUNT;i++) drawSettingsRow(i, i==setIndex);
}

void drawSettingsMenuSelectionOnly(int oldIdx, int newIdx) {
  drawSettingsRow(oldIdx, false);
  drawSettingsRow(newIdx, true);
  drawFooter("UP/DOWN Move  OK Enter", "BACK Menu");
}

// ===== Brightness =====
static void drawBrightnessBar();
void setBrightnessEnter() {
  tft.fillScreen(TH().bg);
  drawHeader("Settings: Brightness");
  drawFooter("L/R Adjust", "BACK Save+Menu");
  clearBody();

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(20, 60);
  tft.print("UI brightness value (BL tied to 3V3 on your wiring).");
  drawBrightnessBar();
}

static void drawBrightnessBar() {
  int x=40,y=120,w=400,h=30;
  tft.fillRoundRect(x, y, w, h, 10, TH().panel);
  tft.drawRoundRect(x, y, w, h, 10, TH().dim);
  int fillW = (brightnessUI * (w-8)) / 255;
  tft.fillRoundRect(x+4, y+4, fillW, h-8, 8, TH().accent);
  tft.setTextColor(TH().text, TH().panel);
  tft.setTextSize(2);
  tft.setCursor(40, 170);
  tft.print("Value: ");
  tft.print(brightnessUI);
}

void setBrightnessLoop(uint32_t now) {
  (void)now;
  bool redraw=false;
  if (bLeft.pressed())  { brightnessUI = max(0, brightnessUI - 8); redraw=true; }
  if (bRight.pressed()) { brightnessUI = min(255, brightnessUI + 8); redraw=true; }
  if (redraw) {
    setBrightnessEnter();
    drawBrightnessBar();
  }
}

// ===== Theme =====
void setThemeEnter() {
  tft.fillScreen(TH().bg);
  drawHeader("Settings: Theme");
  drawFooter("OK Toggle", "BACK Save+Menu");
  clearBody();

  tft.setTextSize(2);
  tft.setTextColor(TH().text, TH().bg);
  tft.setCursor(40, 110);
  tft.print("Theme: ");
  tft.setTextColor(TH().accent, TH().bg);
  tft.print(darkMode ? "Dark" : "Light");

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(40, 150);
  tft.print("OK changes theme instantly.");
}

void setThemeLoop(uint32_t now) {
  (void)now;
  if (bOk.pressed()) {
    darkMode = !darkMode;
    setThemeEnter();
  }
}

// ===== WiFi Captive Portal =====
static WebServer server(80);
static bool wifiPortalRunning = false;
static bool wifiConnectAttempted = false;
static uint32_t lastWifiStatusDraw = 0;

static String htmlEscape(const String& s) {
  String o; o.reserve(s.length());
  for (char c : s) {
    if (c=='&') o += "&amp;";
    else if (c=='<') o += "&lt;";
    else if (c=='>') o += "&gt;";
    else if (c=='"') o += "&quot;";
    else o += c;
  }
  return o;
}

static void startWifiPortal() {
  if (wifiPortalRunning) return;

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("Freslest-Setup");
  IPAddress ip = WiFi.softAPIP();

  server.on("/", HTTP_GET, [ip]() {
    String page;
    page += "<!doctype html><html><head><meta charset='utf-8'/><meta name='viewport' content='width=device-width,initial-scale=1'/>";
    page += "<title>Freslest WiFi Setup</title></head><body style='font-family:Arial;padding:16px'>";
    page += "<h2>Freslest WiFi Setup</h2>";
    page += "<p>Enter SSID and password, then Save.</p>";
    page += "<form method='POST' action='/save'>";
    page += "<label>SSID</label><br/><input name='ssid' style='width:100%;padding:8px'/><br/><br/>";
    page += "<label>Password</label><br/><input name='pass' type='password' style='width:100%;padding:8px'/><br/><br/>";
    page += "<button style='padding:10px 14px'>Save</button>";
    page += "</form>";
    page += "<hr/><p>Device AP: <b>Freslest-Setup</b> | IP: <b>";
    page += ip.toString();
    page += "</b></p>";
    page += "</body></html>";
    server.send(200, "text/html", page);
  });

  server.on("/save", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    settingsSetWifiCreds(ssid, pass);

    String page;
    page += "<!doctype html><html><head><meta charset='utf-8'/><meta name='viewport' content='width=device-width,initial-scale=1'/>";
    page += "<title>Saved</title></head><body style='font-family:Arial;padding:16px'>";
    page += "<h2>Saved!</h2>";
    page += "<p>SSID: <b>" + htmlEscape(ssid) + "</b></p>";
    page += "<p>Now return to device screen. It will try to connect.</p>";
    page += "<a href='/'>Back</a>";
    page += "</body></html>";
    server.send(200, "text/html", page);

    wifiConnectAttempted = false; // trigger reconnect
  });

  server.begin();
  wifiPortalRunning = true;
}

static void stopWifiPortal() {
  if (!wifiPortalRunning) return;
  server.stop();
  WiFi.softAPdisconnect(true);
  wifiPortalRunning = false;
}

static void wifiTryConnect() {
  if (!wifiEnabled) return;
  if (wifiSSID.length() == 0) return;
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPASS.c_str());
  wifiConnectAttempted = true;
}

static void drawWifiScreen() {
  tft.fillScreen(TH().bg);
  drawHeader("Settings: WiFi");
  drawFooter("OK Portal  L Toggle", "BACK Save+Menu");
  clearBody();

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(20, 60);
  tft.print("OK: start/stop setup portal (phone). L: enable/disable WiFi.");

  tft.setTextSize(2);
  tft.setTextColor(TH().text, TH().bg);
  tft.setCursor(20, 95);
  tft.print("WiFi: ");
  tft.setTextColor(TH().accent, TH().bg);
  tft.print(wifiEnabled ? "ENABLED" : "DISABLED");

  tft.setTextSize(1);
  tft.setTextColor(TH().text, TH().bg);
  tft.setCursor(20, 135);
  tft.print("Saved SSID: ");
  tft.print(wifiSSID.length() ? wifiSSID : "(none)");

  tft.setCursor(20, 160);
  tft.print("Portal AP: Freslest-Setup  IP: 192.168.4.1");

  tft.setCursor(20, 185);
  tft.print("Status: ");
  wl_status_t st = WiFi.status();
  tft.setTextColor(TH().accent, TH().bg);
  if (st == WL_CONNECTED) {
    tft.print("CONNECTED ");
    tft.setTextColor(TH().text, TH().bg);
    tft.print(WiFi.localIP());
  } else if (st == WL_CONNECT_FAILED) {
    tft.print("FAILED");
  } else if (st == WL_DISCONNECTED) {
    tft.print("DISCONNECTED");
  } else {
    tft.print("CONNECTING...");
  }

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(20, 220);
  tft.print("How: connect phone WiFi -> Freslest-Setup -> open 192.168.4.1");
}

void setWifiEnter() {
  startWifiPortal(); // portal on entry (user can stop with OK)
  drawWifiScreen();
  lastWifiStatusDraw = 0;
}

void setWifiLoop(uint32_t now) {
  server.handleClient();

  if (bLeft.pressed()) {
    wifiEnabled = !wifiEnabled;
    settingsSave();
    if (!wifiEnabled) {
      WiFi.disconnect(true, true);
    } else {
      wifiConnectAttempted = false;
    }
    drawWifiScreen();
  }

  if (bOk.pressed()) {
    if (wifiPortalRunning) stopWifiPortal();
    else startWifiPortal();
    drawWifiScreen();
  }

  if (wifiEnabled && !wifiConnectAttempted) {
    wifiTryConnect();
  }

  // refresh status text every 500ms
  if (now - lastWifiStatusDraw > 500) {
    lastWifiStatusDraw = now;
    // redraw just status line area
    tft.fillRect(20, 180, 440, 22, TH().bg);
    tft.setTextSize(1);
    tft.setCursor(20, 185);
    tft.setTextColor(TH().text, TH().bg);
    tft.print("Status: ");
    wl_status_t st = WiFi.status();
    tft.setTextColor(TH().accent, TH().bg);
    if (st == WL_CONNECTED) {
      tft.print("CONNECTED ");
      tft.setTextColor(TH().text, TH().bg);
      tft.print(WiFi.localIP());
    } else if (st == WL_CONNECT_FAILED) {
      tft.print("FAILED");
    } else if (st == WL_DISCONNECTED) {
      tft.print("DISCONNECTED");
    } else {
      tft.print("CONNECTING...");
    }
  }
}

// ===== Bluetooth =====
static bool bleInited = false;
static BLEAdvertising* adv = nullptr;

static void bleStart() {
  if (btEnabled == false) return;
  if (!bleInited) {
    BLEDevice::init("FreslestOS");
    bleInited = true;
    adv = BLEDevice::getAdvertising();
    adv->setScanResponse(true);
    adv->setMinPreferred(0x06);
    adv->setMinPreferred(0x12);
  }
  if (adv) adv->start();
}

static void bleStop() {
  if (adv) adv->stop();
}

static void drawBtScreen() {
  tft.fillScreen(TH().bg);
  drawHeader("Settings: Bluetooth");
  drawFooter("OK Toggle", "BACK Save+Menu");
  clearBody();

  tft.setTextSize(2);
  tft.setTextColor(TH().text, TH().bg);
  tft.setCursor(40, 110);
  tft.print("Bluetooth: ");
  tft.setTextColor(TH().accent, TH().bg);
  tft.print(btEnabled ? "ON" : "OFF");

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(40, 150);
  tft.print("When ON: BLE advertising as 'FreslestOS'.");
}

void setBtEnter() {
  drawBtScreen();
  if (btEnabled) bleStart();
  else bleStop();
}

void setBtLoop(uint32_t now) {
  (void)now;
  if (bOk.pressed()) {
    btEnabled = !btEnabled;
    settingsSave();
    if (btEnabled) bleStart();
    else bleStop();
    drawBtScreen();
  }
}

// ===== Flash Reset =====
static int resetSel = 0; // 0 No, 1 Yes
static void drawResetScreen() {
  tft.fillScreen(TH().bg);
  drawHeader("Settings: Flash Reset");
  drawFooter("L/R Select", "OK Confirm  BACK Cancel");
  clearBody();

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(20, 70);
  tft.print("This will reset: theme, brightness, WiFi, Bluetooth, and Note file.");

  int y=120;
  auto drawBtn=[&](int x,const char* txt,bool sel){
    uint16_t fill = sel ? TH().selbg : TH().panel;
    tft.fillRoundRect(x, y, 180, 50, 12, fill);
    tft.drawRoundRect(x, y, 180, 50, 12, sel ? TH().accent : TH().dim);
    tft.setTextSize(2);
    tft.setTextColor(TH().text, fill);
    tft.setCursor(x+55, y+16);
    tft.print(txt);
  };
  drawBtn(60, "NO", resetSel==0);
  drawBtn(240, "YES", resetSel==1);
}

void setFlashResetEnter() {
  resetSel = 0;
  drawResetScreen();
}

void setFlashResetLoop(uint32_t now) {
  (void)now;
  if (bLeft.pressed() || bRight.pressed()) {
    resetSel ^= 1;
    drawResetScreen();
  }
  if (bOk.pressed() && resetSel==1) {
    settingsFactoryReset();
    settingsSave();
    drawResetScreen();
    tft.setTextSize(2);
    tft.setTextColor(TH().accent, TH().bg);
    tft.setCursor(20, 200);
    tft.print("RESET DONE!");
  }
}

// ===== Entry points for main settings menu =====
AppId settingsMenuToApp(int idx) {
  switch(idx){
    case 0: return APP_SET_BRIGHTNESS;
    case 1: return APP_SET_THEME;
    case 2: return APP_SET_WIFI;
    case 3: return APP_SET_BT;
    case 4: return APP_SET_SOUND;
    case 5: return APP_SET_FLASHRESET;
  }
  return APP_SETTINGS;
}

// allow apps.cpp to use setIndex
int& __settingsIndex() { return setIndex; }
int  __settingsCount() { return SET_COUNT; }


// ===== Sound =====
void setSoundEnter() {
  tft.fillScreen(TH().bg);
  drawHeader("Settings: Sound");
  drawFooter("L/R Volume  OK Mute", "BACK Save+Menu");
  clearBody();

  tft.setTextSize(2);
  tft.setTextColor(TH().text, TH().bg);
  tft.setCursor(30, 80);
  tft.print("Sound: ");
  tft.setTextColor(TH().accent, TH().bg);
  tft.print(soundEnabled ? "ON" : "OFF");

  // volume bar
  int x=40,y=140,w=400,h=26;
  tft.fillRoundRect(x, y, w, h, 10, TH().panel);
  tft.drawRoundRect(x, y, w, h, 10, TH().dim);
  int fillW = (soundVolume * (w-8)) / 255;
  tft.fillRoundRect(x+4, y+4, fillW, h-8, 8, TH().accent);

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(40, 180);
  tft.printf("Volume: %d", soundVolume);
  tft.setCursor(40, 200);
  tft.print("Tip: OK toggles mute. L/R changes volume.");
}

void setSoundLoop(uint32_t now) {
  (void)now;
  bool redraw = false;
  if (bLeft.pressed())  { soundVolume = max(0, soundVolume - 10); redraw = true; }
  if (bRight.pressed()) { soundVolume = min(255, soundVolume + 10); redraw = true; }
  if (bOk.pressed())    { soundEnabled = !soundEnabled; redraw = true; }

  if (redraw) {
    settingsSave();
    // apply immediately
    soundApply();
    setSoundEnter();
    // small feedback only if turning ON or adjusting volume (avoid sound when muted)
    if (soundEnabled) audioClick();
  }
}


void setWifiExit() {
  stopWifiPortal();
}

void setBtExit() {
  // keep BLE state according to btEnabled; stopping saves power when leaving page.
  if (!btEnabled) bleStop();
}
