#include "apps.h"

#include "gfx.h"
#include "input.h"
#include "settings.h"
#include "audio_manager.h"
#include "game.h"

#include <algorithm>

AppId app = APP_MENU;

// ======================= MENU ITEMS =======================
static const char* MENU_ITEMS[] = { "Apps", "Settings", "System Info", "Test", "Game" };
static constexpr int MENU_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);
static int menuIndex = 0;

// ======================= TASK SWITCHER =======================
static constexpr uint32_t OK_HOLD_MS = 650;
static constexpr int MAX_TASKS = 5;

static AppId tasks[MAX_TASKS];
static int taskCount = 0;
static int taskSel = 0;
static bool taskView = false;

// ======================= FPS =======================
uint32_t fpsValue = 0;
static uint32_t fpsLast = 0;
static uint32_t fpsCount = 0;
static bool footerDirty = true;

void fpsInit() {
  fpsLast = millis();
  fpsCount = 0;
  fpsValue = 0;
  footerDirty = true;
}

void fpsTick(uint32_t now) {
  fpsCount++;
  if (now - fpsLast >= 1000) {
    fpsValue = fpsCount;
    fpsCount = 0;
    fpsLast = now;
    footerDirty = true;

    if (!taskView && (app == APP_MENU || app == APP_APPS || app == APP_SETTINGS || app == APP_SYSINFO || app == APP_TEST)) {
      if (app == APP_MENU)     drawFooter("UP/DOWN Move  OK Enter", "HOLD OK Tasks");
      if (app == APP_APPS)     drawFooter("OK Beep", "BACK Menu");
      if (app == APP_SETTINGS) drawFooter("L/R Bright(UI) OK Theme", "BACK Save+Menu");
      if (app == APP_SYSINFO)  drawFooter("OK Refresh", "BACK Menu");
      if (app == APP_TEST)     drawFooter("Press buttons", "BACK Menu");
    }
  }
}

// ======================= TASK HELPERS =======================
static bool isInTasksLocal(AppId a) {
  for (int i = 0; i < taskCount; i++) if (tasks[i] == a) return true;
  return false;
}
static void addTaskLocal(AppId a) {
  if (a == APP_MENU) return;
  if (isInTasksLocal(a)) return;
  if (taskCount < MAX_TASKS) tasks[taskCount++] = a;
}
static void removeTaskAt(int idx) {
  if (idx < 0 || idx >= taskCount) return;
  for (int i = idx; i < taskCount - 1; i++) tasks[i] = tasks[i + 1];
  taskCount--;
  if (taskSel >= taskCount) taskSel = std::max(0, taskCount - 1);
}

// ======================= DRAW HELPERS =======================
void drawHeader(const char* title) {
  tft.fillRect(0, 0, 480, 44, TH().panel);
  tft.setTextColor(TH().text, TH().panel);
  tft.setTextSize(2);
  tft.setCursor(12, 12);
  tft.print("FRESLEST OS");
  tft.setTextSize(1);
  tft.setCursor(12, 30);
  tft.print(title);
}

void drawFooter(const char* leftHint, const char* rightHint) {
  tft.fillRect(0, 296, 480, 24, TH().panel);
  tft.setTextColor(TH().text, TH().panel);
  tft.setTextSize(1);
  tft.setCursor(10, 302);
  tft.print(leftHint);

  tft.setCursor(200, 302);
  tft.print("FPS:");
  tft.print(fpsValue);

  tft.setCursor(330, 302);
  tft.print(rightHint);

  footerDirty = false;
}

void clearBody() {
  tft.fillRect(0, 44, 480, 252, TH().bg);
}

// ======================= BOOT =======================
void bootScreen() {
  tft.fillScreen(TH().bg);
  tft.setTextColor(TH().text, TH().bg);
  tft.setTextSize(3);
  tft.setCursor(60, 115);
  tft.print("FRESLEST OS");
  tft.setTextSize(1);
  tft.setCursor(60, 160);
  tft.print("ULTRA PERF | Direct Draw");
  tft.setCursor(60, 185);
  tft.print(psramFound() ? "PSRAM: OK" : "PSRAM: NOT FOUND!");
  delay(450);
}

// ======================= MENU (partial redraw) =======================
static constexpr int MENU_TOPY = 60;
static constexpr int MENU_ITEMH = 42;

static void drawMenuRow(int i, bool selected) {
  int y = MENU_TOPY + i * MENU_ITEMH;
  tft.fillRect(18, y - 2, 444, 40, TH().bg);

  tft.setTextSize(2);
  if (selected) {
    tft.fillRoundRect(20, y, 440, 36, 8, TH().selbg);
    tft.drawRoundRect(20, y, 440, 36, 8, TH().accent);
    tft.setTextColor(TH().text, TH().selbg);
    tft.setCursor(40, y + 9);
    tft.print("> ");
    tft.print(MENU_ITEMS[i]);
  } else {
    tft.setTextColor(TH().dim, TH().bg);
    tft.setCursor(40, y + 9);
    tft.print("  ");
    tft.print(MENU_ITEMS[i]);
  }
}

void drawMenuFull() {
  tft.fillScreen(TH().bg);
  drawHeader("Main Menu");
  drawFooter("UP/DOWN Move  OK Enter", "HOLD OK Tasks");
  for (int i = 0; i < MENU_COUNT; i++) drawMenuRow(i, i == menuIndex);
}

void drawMenuSelectionOnly(int oldIdx, int newIdx) {
  drawMenuRow(oldIdx, false);
  drawMenuRow(newIdx, true);
  if (footerDirty) drawFooter("UP/DOWN Move  OK Enter", "HOLD OK Tasks");
}


static const char* appName(AppId a) {
  switch(a){
    case APP_APPS: return "Apps";
    case APP_SETTINGS: return "Settings";
    case APP_SYSINFO: return "System";
    case APP_TEST: return "Test";
    case APP_GAME_MENU: return "Game";
    case APP_CALC: return "Calc";
    case APP_NOTE: return "Note";
    case APP_PAINT: return "Paint";
    case APP_CLOCK: return "Clock";
    case APP_SET_BRIGHTNESS: return "Bright";
    case APP_SET_THEME: return "Theme";
    case APP_SET_WIFI: return "WiFi";
    case APP_SET_BT: return "BT";
    case APP_SET_FLASHRESET: return "Reset";
    case APP_DINO: return "Dino";
    case APP_TETRIS: return "Tetris";
    case APP_SNAKE: return "Snake";
    case APP_MINES: return "Mines";
    case APP_2048: return "2048";
    default: return "App";
  }
}

// ======================= TASK SWITCHER UI =======================
static void drawTaskSwitcherFull() {
  tft.fillScreen(TH().bg);
  drawHeader("Task Switcher");
  drawFooter("L/R Select  OK Switch", "UP Close  BACK Exit");

  tft.setTextSize(2);

  if (taskCount == 0) {
    tft.setTextColor(TH().text, TH().bg);
    tft.setCursor(60, 140);
    tft.print("No apps running");
    tft.setTextSize(1);
    tft.setCursor(60, 170);
    tft.print("Open an app from menu.");
    return;
  }

  // 3 apps per page
  static constexpr int PER_PAGE = 3;
  int totalPages = (taskCount + PER_PAGE - 1) / PER_PAGE;
  int page = taskSel / PER_PAGE;
  int start = page * PER_PAGE;
  int end = start + PER_PAGE;
  if (end > taskCount) end = taskCount;

  int cardW = 140, cardH = 90;
  int gap = 10;
  int shown = end - start;
  int totalW = shown * cardW + (shown - 1) * gap;
  int startX = (480 - totalW) / 2;
  int y = 110;

  for (int i = start; i < end; i++) {
    int local = i - start;
    int x = startX + local * (cardW + gap);
    bool sel = (i == taskSel);
    uint16_t fill = sel ? TH().selbg : TH().panel;

    tft.fillRoundRect(x, y, cardW, cardH, 14, fill);
    tft.drawRoundRect(x, y, cardW, cardH, 14, sel ? TH().accent : TH().dim);

    tft.setTextColor(TH().text, fill);
    tft.setCursor(x + 12, y + 18);
    tft.setTextSize(2);
    tft.print(appName(tasks[i]));

    tft.setTextSize(1);
    tft.setCursor(x + 12, y + 52);
    tft.print("OK to switch");
  }

  // page dots
  tft.setTextSize(2);
  int dotsY = 250;
  int dotsW = totalPages * 14;
  int dotsX = (480 - dotsW) / 2;
  for (int p = 0; p < totalPages; p++) {
    uint16_t c = (p == page) ? TH().accent : TH().dim;
    tft.fillCircle(dotsX + p * 14, dotsY, 4, c);
  }

  tft.setTextSize(1);
  tft.setTextColor(TH().dim, TH().bg);
  tft.setCursor(20, 270);
  tft.print("Tip: UP closes selected task.");
}

void taskSwitcherHandle(uint32_t now) {
  static uint32_t okDownAt = 0;

  // detect hold
  if (bOk.down()) {
    if (okDownAt == 0) okDownAt = now;
    if (!taskView && (now - okDownAt >= OK_HOLD_MS)) {
      taskView = true;
      taskSel = std::min(taskSel, std::max(0, taskCount - 1));
      drawTaskSwitcherFull(); audioClick();
    }
  } else {
    okDownAt = 0;
  }

  if (!taskView) return;

  bool changed = false;

  if (bLeft.pressed() && taskCount > 0)  { taskSel = (taskSel - 1 + taskCount) % taskCount; changed = true; audioClick(); }
  if (bRight.pressed() && taskCount > 0) { taskSel = (taskSel + 1) % taskCount; changed = true; audioClick(); }

  if (bUp.pressed() && taskCount > 0) {
    AppId closing = tasks[taskSel];
    removeTaskAt(taskSel);
    if (app == closing) app = APP_MENU;
    changed = true; audioClick();
  }

  if (bOk.pressed() && taskCount > 0) {
    AppId target = tasks[taskSel];
    audioClick();
    taskView = false;
    if (target == APP_MENU) drawMenuFull();
    else enterApp(target);
  }

  if (bBack.pressed()) {
    audioClick();
    taskView = false;
    if (app == APP_MENU) drawMenuFull();
    else enterApp(app);
  }

  if (changed) { drawTaskSwitcherFull(); audioClick(); }
}

// ======================= APPS =======================
void drawApps() {
  drawAppsMenuFull();
}

void drawSettings() {
  drawSettingsMenuFull();
}

void drawSysInfo() {
  tft.fillScreen(TH().bg);
  drawHeader("System Info");
  drawFooter("OK Refresh", "BACK Menu");
  clearBody();

  tft.setTextColor(TH().text, TH().bg);
  tft.setTextSize(1);

  int y = 70;
  tft.setCursor(20, y); tft.printf("CPU: %u MHz", getCpuFrequencyMhz()); y += 18;
  tft.setCursor(20, y); tft.printf("Flash: %u bytes", ESP.getFlashChipSize()); y += 18;
  tft.setCursor(20, y); tft.printf("Heap Free: %u bytes", ESP.getFreeHeap()); y += 18;

#if defined(BOARD_HAS_PSRAM)
  tft.setCursor(20, y); tft.printf("PSRAM Found: %s", psramFound() ? "YES" : "NO"); y += 18;
  tft.setCursor(20, y); tft.printf("PSRAM Free: %u bytes", ESP.getFreePsram()); y += 18;
#else
  tft.setCursor(20, y); tft.print("PSRAM: (board flag off)"); y += 18;
#endif

  tft.setCursor(20, y); tft.printf("Theme: %s", darkMode ? "Dark" : "Light"); y += 18;
  tft.setCursor(20, y); tft.printf("Brightness(UI): %d", brightnessUI); y += 18;
}

void drawTestBase() {
  tft.fillScreen(TH().bg);
  drawHeader("Test");
  drawFooter("Press buttons", "BACK Menu");
  clearBody();

  tft.setTextColor(TH().text, TH().bg);
  tft.setTextSize(2);
  tft.setCursor(20, 70);
  tft.print("Buttons live:");

  tft.fillRoundRect(20, 120, 440, 150, 12, TH().panel);
}

void drawTestStatesOnly() {
  tft.fillRect(30, 130, 420, 130, TH().panel);
  tft.setTextColor(TH().text, TH().panel);
  tft.setTextSize(2);
  tft.setCursor(40, 140);
  tft.printf("UP:%d  DOWN:%d", bUp.down(), bDown.down());
  tft.setCursor(40, 170);
  tft.printf("LT:%d  RT:%d", bLeft.down(), bRight.down());
  tft.setCursor(40, 200);
  tft.printf("OK:%d  BK:%d", bOk.down(), bBack.down());
  tft.setTextSize(1);
  tft.setCursor(40, 235);
  tft.printf("FPS:%u", fpsValue);
}

// ======================= NAV =======================
AppId menuToApp(int idx) {
  switch (idx) {
    case 0: return APP_APPS;
    case 1: return APP_SETTINGS;
    case 2: return APP_SYSINFO;
    case 3: return APP_TEST;
    default: return APP_GAME;
  }
}

void enterApp(AppId next) {
  // Normalize: main Settings opens settings menu; main Game opens game menu
  if (next == APP_SETTINGS) next = APP_SETTINGS;
  if (next == APP_GAME) next = APP_GAME_MENU;

  app = next;
  addTaskLocal(app);
  footerDirty = true;

  switch (app) {
    case APP_APPS:        drawAppsMenuFull(); break;
    case APP_SETTINGS:    drawSettingsMenuFull(); break;
    case APP_SYSINFO:     drawSysInfo(); break;
    case APP_TEST:        { drawTestBase(); drawTestStatesOnly(); } break;

    case APP_GAME_MENU:   drawGameMenuFull(); break;

    case APP_CALC:        calcEnter(); break;
    case APP_NOTE:        noteEnter(); break;
    case APP_PAINT:       paintEnter(); break;
    case APP_CLOCK:       clockEnter(); break;

    case APP_SET_BRIGHTNESS: setBrightnessEnter(); break;
    case APP_SET_THEME:      setThemeEnter(); break;
    case APP_SET_WIFI:       setWifiEnter(); break;
    case APP_SET_BT:         setBtEnter(); break;
    case APP_SET_SOUND:      setSoundEnter(); break;
    case APP_SET_FLASHRESET: setFlashResetEnter(); break;

    case APP_DINO:        dinoEnter(); break;
    case APP_TETRIS:      tetrisEnter(); break;
    case APP_SNAKE:       snakeEnter(); break;
    case APP_MINES:       minesEnter(); break;
    case APP_2048:        g2048Enter(); break;

    default: break;
  }
}

void backToMenu() {
  // cleanup for pages that keep services running
  if (app == APP_SET_WIFI) setWifiExit();
  if (app == APP_SET_BT)   setBtExit();

  app = APP_MENU;
  footerDirty = true;
  drawMenuFull();
}

// ======================= MAIN APP LOOP =======================
void appLoop(uint32_t now) {
  // BACK navigation tree:
  if (bBack.pressed()) {
    // exit services if needed
    if (app == APP_SET_WIFI) setWifiExit();
    if (app == APP_SET_BT)   setBtExit();

    // From sub-apps -> parent menu
    if (app == APP_CALC || app == APP_NOTE || app == APP_PAINT || app == APP_CLOCK) {
      enterApp(APP_APPS);
      return;
    }
    if (app == APP_SET_BRIGHTNESS || app == APP_SET_THEME || app == APP_SET_WIFI || app == APP_SET_BT || app == APP_SET_FLASHRESET) {
      settingsSave();
      enterApp(APP_SETTINGS);
      return;
    }
    if (app == APP_DINO || app == APP_TETRIS || app == APP_SNAKE || app == APP_MINES || app == APP_2048) {
      enterApp(APP_GAME_MENU);
      return;
    }

    // Parent pages -> main menu
    backToMenu();
    return;
  }

  // MAIN MENU
  if (app == APP_MENU) {
    if (bUp.pressed()) {
      int old = menuIndex;
      menuIndex = (menuIndex - 1 + MENU_COUNT) % MENU_COUNT;
      drawMenuSelectionOnly(old, menuIndex);
    }
    if (bDown.pressed()) {
      int old = menuIndex;
      menuIndex = (menuIndex + 1) % MENU_COUNT;
      drawMenuSelectionOnly(old, menuIndex);
    }
    if (bOk.pressed()) {
      enterApp(menuToApp(menuIndex));
    }
    return;
  }

  // APPS MENU
  if (app == APP_APPS) {
    int& idx = __appsIndex();
    int cnt = __appsCount();
    if (bUp.pressed()) {
      int old = idx;
      idx = (idx - 1 + cnt) % cnt;
      drawAppsMenuSelectionOnly(old, idx);
    }
    if (bDown.pressed()) {
      int old = idx;
      idx = (idx + 1) % cnt;
      drawAppsMenuSelectionOnly(old, idx);
    }
    if (bOk.pressed()) {
      enterApp(__appsMenuToApp(idx));
    }
    return;
  }

  // SETTINGS MENU
  if (app == APP_SETTINGS) {
    int& idx = __settingsIndex();
    if (bUp.pressed()) {
      int old = idx;
      int cnt = __settingsCount();
      idx = (idx - 1 + cnt) % cnt;
      drawSettingsMenuSelectionOnly(old, idx);
    }
    if (bDown.pressed()) {
      int old = idx;
      int cnt = __settingsCount();
      idx = (idx + 1) % cnt;
      drawSettingsMenuSelectionOnly(old, idx);
    }
    if (bOk.pressed()) {
      enterApp(settingsMenuToApp(idx));
    }
    return;
  }

  // GAME MENU
  if (app == APP_GAME_MENU) {
    int& idx = __gameIndex();
    int cnt = __gameCount();
    if (bUp.pressed()) {
      int old = idx;
      idx = (idx - 1 + cnt) % cnt;
      drawGameMenuSelectionOnly(old, idx);
    }
    if (bDown.pressed()) {
      int old = idx;
      idx = (idx + 1) % cnt;
      drawGameMenuSelectionOnly(old, idx);
    }
    if (bOk.pressed()) {
      enterApp(__gameMenuToApp(idx));
    }
    return;
  }

  // SYSINFO
  if (app == APP_SYSINFO) {
    if (bOk.pressed()) drawSysInfo();
    return;
  }

  // TEST
  if (app == APP_TEST) {
    drawTestStatesOnly();
    return;
  }

  // Sub-apps
  if (app == APP_CALC)  { calcLoop(now); return; }
  if (app == APP_NOTE)  { noteLoop(now); return; }
  if (app == APP_PAINT) { paintLoop(now); return; }
  if (app == APP_CLOCK) { clockLoop(now); return; }

  // Settings pages
  if (app == APP_SET_BRIGHTNESS) { setBrightnessLoop(now); return; }
  if (app == APP_SET_THEME)      { setThemeLoop(now); return; }
  if (app == APP_SET_WIFI)       { setWifiLoop(now); return; }
  if (app == APP_SET_BT)         { setBtLoop(now); return; }
  if (app == APP_SET_FLASHRESET) { setFlashResetLoop(now); return; }

  // Games
  if (app == APP_DINO)   { dinoLoop(now); return; }
  if (app == APP_TETRIS) { tetrisLoop(now); return; }
  if (app == APP_SNAKE)  { snakeLoop(now); return; }
  if (app == APP_MINES)  { minesLoop(now); return; }
  if (app == APP_2048)   { g2048Loop(now); return; }
}
