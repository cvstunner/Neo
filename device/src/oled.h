#pragma once
// ═══════════════════════════════════════════════════════════
//  oled.h — All display drawing, animations, dashboard pages
// ═══════════════════════════════════════════════════════════
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;
extern SysConfig sysCfg;
extern WeatherData weather;
extern Profile profiles[];
extern int profileCount;
extern Task tasks[];
extern int taskCount;
extern String lastUID;
extern unsigned long lastScanMs;
extern bool wifiConnected;
extern bool timeReady;
extern int moisturePct;

// ── primitives ───────────────────────────────────────────────
inline void oled() { display.display(); }
inline void cls() { display.clearDisplay(); }
inline void hline(int y) {
  display.drawLine(0, y, SCREEN_WIDTH - 1, y, SSD1306_WHITE);
}
inline void vline(int x, int y1, int y2) {
  display.drawLine(x, y1, x, y2, SSD1306_WHITE);
}

void txt(int x, int y, uint8_t sz, const char *s, bool inv = false) {
  display.setTextSize(sz);
  display.setTextColor(inv ? SSD1306_BLACK : SSD1306_WHITE);
  display.setCursor(x, y);
  display.print(s);
}
void txt(int x, int y, uint8_t sz, const String &s, bool inv = false) {
  txt(x, y, sz, s.c_str(), inv);
}

void topBar(const char *left, const char *right = "") {
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  txt(2, 2, 1, left, true);
  if (strlen(right)) {
    int rw = strlen(right) * 6;
    txt(126 - rw, 2, 1, right, true);
  }
}

// ── wifi signal bars (4 bars, x=start, y=bottom) ─────────────
void wifiIcon(int bx, int by) {
  if (!wifiConnected) {
    txt(bx, by - 7, 1, "?");
    return;
  }
  int rssi = WiFi.RSSI();
  int bars = rssi > -55 ? 4 : rssi > -65 ? 3 : rssi > -75 ? 2 : 1;
  for (int b = 0; b < 4; b++) {
    int bh = (b + 1) * 3;
    int bx2 = bx + b * 5, by2 = by - bh;
    if (b < bars)
      display.fillRect(bx2, by2, 4, bh, SSD1306_WHITE);
    else
      display.drawRect(bx2, by2, 4, bh, SSD1306_WHITE);
  }
}

// ── small clock string ────────────────────────────────────────
String timeStr(bool secs = false) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  char buf[12];
  if (secs)
    strftime(buf, sizeof(buf), "%H:%M:%S", t);
  else
    strftime(buf, sizeof(buf), "%I:%M %p", t);
  return String(buf);
}
String dateStr() {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  char buf[22];
  strftime(buf, sizeof(buf), "%a, %d %b %Y", t);
  return String(buf);
}
String shortTime() {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  char buf[8];
  strftime(buf, sizeof(buf), "%H:%M", t);
  return String(buf);
}

// ═══════════════════════════════════════════════
//  BOOT ANIMATION
// ═══════════════════════════════════════════════
void drawNEOLogo(int ox, int oy) {
  // N — thick strokes
  display.fillRect(ox, oy, 5, 22, SSD1306_WHITE);
  display.fillRect(ox + 14, oy, 5, 22, SSD1306_WHITE);
  // diagonal of N
  for (int i = 0; i < 5; i++) {
    display.drawLine(ox + i, oy, ox + 14 + i, oy + 22, SSD1306_WHITE);
  }
  // E
  int ex = ox + 22;
  display.fillRect(ex, oy, 5, 22, SSD1306_WHITE);
  display.fillRect(ex + 4, oy, 14, 4, SSD1306_WHITE);
  display.fillRect(ex + 4, oy + 9, 10, 4, SSD1306_WHITE);
  display.fillRect(ex + 4, oy + 18, 14, 4, SSD1306_WHITE);
  // O
  int ox2 = ox + 46;
  display.drawRoundRect(ox2, oy, 18, 22, 5, SSD1306_WHITE);
  display.drawRoundRect(ox2 + 1, oy + 1, 16, 20, 4, SSD1306_WHITE);
  display.drawRoundRect(ox2 + 2, oy + 2, 14, 18, 3, SSD1306_WHITE);
}

void bootAnimation() {
  // 1. scanline sweep
  for (int i = 0; i <= SCREEN_HEIGHT; i += 2) {
    cls();
    display.fillRect(0, 0, 128, i, SSD1306_WHITE);
    oled();
    delay(6);
  }
  delay(60);
  // 2. invert out
  for (int i = SCREEN_HEIGHT; i >= 0; i -= 4) {
    cls();
    display.fillRect(0, 0, 128, i, SSD1306_WHITE);
    oled();
    delay(8);
  }
  // 3. NEO slides in from left
  for (int x = -55; x <= 14; x += 5) {
    cls();
    drawNEOLogo(x, 20);
    oled();
    delay(16);
  }
  // 4. tagline types in
  cls();
  drawNEOLogo(14, 20);
  const char *tag = "Here for You";
  String built = "";
  for (int i = 0; tag[i]; i++) {
    built += tag[i];
    cls();
    drawNEOLogo(14, 20);
    int tw = built.length() * 6;
    txt((128 - tw) / 2, 48, 1, built);
    oled();
    delay(55);
  }
  // 5. pulse border 3x
  for (int p = 0; p < 3; p++) {
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    display.drawRect(1, 1, 126, 62, SSD1306_WHITE);
    oled();
    delay(180);
    display.drawRect(0, 0, 128, 64, SSD1306_BLACK);
    display.drawRect(1, 1, 126, 62, SSD1306_BLACK);
    oled();
    delay(120);
  }
  delay(600);
}

// ═══════════════════════════════════════════════
//  WIFI PORTAL SCREENS
// ═══════════════════════════════════════════════
void showPortalScreen(const String &ip) {
  cls();
  display.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);
  topBar("  WiFi Setup");
  txt(4, 16, 1, "1. Join WiFi:");
  txt(4, 26, 1, "   NEO-Setup");
  hline(38);
  txt(4, 42, 1, "2. Open browser:");
  txt(4, 52, 1, "   " + ip);
  oled();
}

void showConnecting(const String &ssid, int dot) {
  cls();
  topBar("Connecting...");
  txt(2, 16, 1, ssid.length() > 18 ? ssid.substring(0, 18) : ssid);
  // animated dots
  String d = "";
  for (int i = 0; i < dot % 4; i++)
    d += ".";
  txt(2, 30, 1, "Please wait" + d);
  // spinning arc
  static int ang = 0;
  ang = (ang + 30) % 360;
  int cx = 110, cy = 46, r = 10;
  for (int a = ang; a < ang + 240; a += 15) {
    float rad = a * 3.14159 / 180;
    display.drawPixel(cx + r * cos(rad), cy + r * sin(rad), SSD1306_WHITE);
  }
  display.fillCircle(cx, cy, 3, SSD1306_WHITE);
  oled();
}

void showWifiSuccess(const String &name, const String &ip) {
  cls();
  // fill top half white
  display.fillRect(0, 0, 128, 32, SSD1306_WHITE);
  drawNEOLogo(30, 5); // small NEO in black on white
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(30, 5);
  // just draw simple connected text
  txt(20, 36, 1, "Connected!");
  txt(4, 48, 1, name.length() ? "Hi " + name + "!" : "Welcome!");
  txt(4, 56, 1, ip);
  oled();
  delay(2500);
}

// ═══════════════════════════════════════════════
//  RFID SCAN ANIMATION
// ═══════════════════════════════════════════════
void animateScan(const String &uid, const String &who) {
  // flash 3x
  for (int i = 0; i < 3; i++) {
    cls();
    display.drawRoundRect(1, 1, 126, 62, 5, SSD1306_WHITE);
    txt(22, 8, 1, "CARD SCANNED");
    hline(20);
    txt(4, 26, 1, "UID: " + uid);
    if (who.length())
      txt(4, 40, 1, ">>> " + who);
    oled();
    delay(140);
    cls();
    oled();
    delay(70);
  }
  // solid hold
  cls();
  display.fillRoundRect(1, 1, 126, 62, 5, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(22, 8);
  display.print("CARD SCANNED");
  display.drawLine(1, 20, 126, 20, SSD1306_BLACK);
  display.setCursor(4, 26);
  display.print("UID: " + uid);
  if (who.length()) {
    display.setCursor(4, 40);
    display.print(">>> " + who);
  }
  display.setTextColor(SSD1306_WHITE);
  oled();
  delay(1800);
}

// ═══════════════════════════════════════════════
//  DASHBOARD PAGES
// ═══════════════════════════════════════════════

// ── Page 0: Clock ────────────────────────────────────────────
void pageClock() {
  cls();
  topBar("NEO", sysCfg.ownerName.length()
                    ? sysCfg.ownerName.substring(0, 10).c_str()
                    : "Dashboard");
  // big time
  String t = timeStr();
  display.setTextSize(2);
  int tw = t.length() * 12;
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((128 - tw) / 2, 16);
  display.print(t);
  // date
  display.setTextSize(1);
  String d = dateStr();
  int dw = d.length() * 6;
  display.setCursor((128 - dw) / 2, 38);
  display.print(d);
  hline(50);
  // bottom: wifi bars + pending tasks count
  wifiIcon(2, 62);
  int pending = 0;
  for (int i = 0; i < taskCount; i++)
    if (!tasks[i].done)
      pending++;
  if (pending > 0)
    txt(32, 54, 1,
        String(pending) + " task" + (pending > 1 ? "s" : "") + " due");
  else
    txt(32, 54, 1, "No pending tasks");
  oled();
}

// ── Page 1: Weather ──────────────────────────────────────────
void pageWeather() {
  cls();
  String loc =
      sysCfg.location.length() ? sysCfg.location.substring(0, 12) : "--";
  topBar("Weather", loc.c_str());
  if (weather.code < 0) {
    txt(10, 28, 1, "Fetching weather...");
    oled();
    return;
  }
  // big temp left
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 16);
  display.print(String((int)weather.temp));
  display.setTextSize(1);
  display.setCursor(4 + String((int)weather.temp).length() * 18, 16);
  display.print("oC");
  // feels like
  txt(4, 42, 1, "Feels " + String((int)weather.feelsLike) + (char)247 + "C");
  // condition right
  txt(72, 16, 1, weather.desc);
  // humidity & wind
  txt(72, 28, 1, "H:" + String(weather.humidity) + "%");
  txt(72, 38, 1, "W:" + String((int)weather.windSpeed) + "km/h");
  // AQI
  hline(52);
  if (weather.aqi > 0) {
    String aq = "AQI:" + String(weather.aqi) + " ";
    if (weather.aqi <= 50)
      aq += "Good";
    else if (weather.aqi <= 100)
      aq += "Moderate";
    else if (weather.aqi <= 150)
      aq += "Unhealthy";
    else
      aq += "Poor";
    txt(2, 55, 1, aq);
  } else {
    txt(2, 55, 1, "AQI: --");
  }
  oled();
}

// ── Page 2: Tasks ────────────────────────────────────────────
void pageTasks() {
  cls();
  topBar("Tasks & Reminders");
  if (taskCount == 0) {
    txt(10, 28, 1, "No tasks yet.");
    txt(10, 40, 1, "Add via web portal.");
    oled();
    return;
  }
  int y = 15, shown = 0;
  for (int i = 0; i < taskCount && shown < 4; i++) {
    if (tasks[i].done)
      continue;
    // checkbox
    if (tasks[i].done)
      display.fillRect(2, y, 7, 7, SSD1306_WHITE);
    else
      display.drawRect(2, y, 7, 7, SSD1306_WHITE);
    // owner tag
    String line = tasks[i].text;
    if (line.length() > 16)
      line = line.substring(0, 15) + "~";
    txt(12, y, 1, line);
    // time if set
    if (tasks[i].dueTime.length()) {
      txt(100, y, 1, tasks[i].dueTime);
    }
    y += 12;
    shown++;
  }
  if (taskCount - shown > 0) {
    hline(60);
    txt(2, 56, 1, "+" + String(taskCount - shown) + " more");
  }
  oled();
}

// ── Page 3: Profiles / Who's home ────────────────────────────
void pageProfiles() {
  cls();
  topBar("People & Plant");
  if (profileCount == 0) {
    txt(4, 20, 1, "No profiles set up.");
    txt(4, 32, 1, "Scan cards & assign");
    txt(4, 44, 1, "via web portal.");
    oled();
    return;
  }
  int y = 15;
  for (int i = 0; i < profileCount && i < 4; i++) {
    Profile &p = profiles[i];
    // role icon
    char icon = p.role == "plant"   ? '*'
                : p.role == "owner" ? '>'
                                    : (p.role == "partner" ? '<' : '-');
    txt(2, y, 1, String(icon));
    txt(12, y, 1, p.name.length() ? p.name : "Unknown");
    if (p.lastSeenTime.length()) {
      String ago = p.lastSeenTime.substring(0, 5);
      txt(86, y, 1, ago);
    } else {
      txt(86, y, 1, "--:--");
    }
    // moisture for plant
    if (p.role == "plant" && MOISTURE_ENABLED) {
      txt(60, y, 1, String(moisturePct) + "%");
    }
    y += 12;
  }
  hline(y + 1);
  txt(2, y + 4, 1, "Last: " + lastUID.substring(0, 14));
  oled();
}

// ── Page 4: Plant ────────────────────────────────────────────
void pagePlant() {
  cls();
  topBar("Plant Care");
  if (!MOISTURE_ENABLED) {
    txt(4, 20, 1, "No sensor wired.");
    txt(4, 32, 1, "Add moisture sensor");
    txt(4, 44, 1, "to pin A0.");
    oled();
    return;
  }
  // big moisture %
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  String pct = String(moisturePct) + "%";
  display.setCursor((128 - pct.length() * 18) / 2, 14);
  display.print(pct);
  // bar
  display.drawRect(4, 42, 120, 10, SSD1306_WHITE);
  int barW = map(moisturePct, 0, 100, 0, 118);
  display.fillRect(5, 43, barW, 8, SSD1306_WHITE);
  // status
  display.setTextSize(1);
  String status;
  if (moisturePct < 20)
    status = "Needs water NOW!";
  else if (moisturePct < 40)
    status = "Getting dry";
  else if (moisturePct < 70)
    status = "Healthy";
  else
    status = "Well watered";
  int sw = status.length() * 6;
  display.setCursor((128 - sw) / 2, 56);
  display.print(status);
  oled();
}

// ── Page 5: System ───────────────────────────────────────────
void pageSystem() {
  cls();
  topBar("System");
  txt(2, 15, 1, "IP: " + (wifiConnected ? WiFi.localIP().toString() : "--"));
  txt(2, 26, 1, "Signal: " + String(wifiConnected ? WiFi.RSSI() : 0) + " dBm");
  txt(2, 37, 1, "Heap: " + String(ESP.getFreeHeap() / 1024) + " KB free");
  txt(2, 48, 1, "Up: " + String(millis() / 60000) + " min");
  wifiIcon(100, 62);
  oled();
}

// ── Page dispatcher ──────────────────────────────────────────
int _dashPage = 0;
int _totalPages = MOISTURE_ENABLED ? 6 : 5;

void showDashboard(bool force = false) {
  switch (_dashPage % _totalPages) {
  case 0:
    pageClock();
    break;
  case 1:
    pageWeather();
    break;
  case 2:
    pageTasks();
    break;
  case 3:
    pageProfiles();
    break;
  case 4:
    pagePlant();
    break;
  case 5:
    pageSystem();
    break;
  }
}

void nextDashPage() {
  _dashPage++;
  showDashboard();
}