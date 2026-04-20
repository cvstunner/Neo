/*
 * ╔═══════════════════════════════════════════════════════╗
 * ║  NEO — Here for You                                   ║
 * ║  NodeMCU v1 (ESP8266) · SSD1306 OLED · MFRC522 RFID  ║
 * ╠═══════════════════════════════════════════════════════╣
 * ║  OLED (SW SPI): MOSI=D3 CLK=D4 DC=D2 CS=D1 RST=3V3  ║
 * ║  RFID (HW SPI): SS=D8  RST=D0 MOSI=D7 MISO=D6 SCK=D5║
 * ║  Plant sensor : A0 (optional, set MOISTURE_ENABLED)  ║
 * ╚═══════════════════════════════════════════════════════╝
 *
 * lib_deps:
 *   adafruit/Adafruit SSD1306@^2.5.16
 *   miguelbalboa/MFRC522@^1.4.12
 *   bblanchon/ArduinoJson@^7.0.0
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <time.h>

#include "config.h"

// ── Hardware instances ────────────────────────────────────────
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK,
                         OLED_DC, OLED_RST, OLED_CS);
MFRC522 rfid(RFID_SS, RFID_RST);
ESP8266WebServer server(80);

// ── Global state ──────────────────────────────────────────────
SysConfig sysCfg;
WeatherData weather;
Profile profiles[MAX_PROFILES];
int profileCount = 0;
Task tasks[MAX_TASKS];
int taskCount = 0;

bool wifiConnected = false;
bool timeReady = false;
bool serverRunning = false;
bool portalMode = false;

String lastUID = "";
String lastWho = "";
String lastScanTime = "";
unsigned long lastScanMs = 0;

int moisturePct = -1;
unsigned long lastMoistureMs = 0;

unsigned long lastPageMs = 0;
unsigned long lastWeatherMs = 0;

// Include UI modules (after globals so externs resolve)
#include "oled.h"
#include "portal.h"

// ═══════════════════════════════════════════════
//  CITY → LAT/LON (open-meteo needs coords)
// ═══════════════════════════════════════════════
void cityCoords(const String &city, float &lat, float &lon) {
  String c = city;
  c.toLowerCase();
  struct {
    const char *k;
    float la, lo;
  } lut[] = {{"mumbai", 19.08, 72.88},    {"bombay", 19.08, 72.88},
             {"delhi", 28.61, 77.21},     {"new delhi", 28.61, 77.21},
             {"bangalore", 12.97, 77.59}, {"bengaluru", 12.97, 77.59},
             {"hyderabad", 17.38, 78.49}, {"chennai", 13.08, 80.27},
             {"pune", 18.52, 73.86},      {"kolkata", 22.57, 88.36},
             {"ahmedabad", 23.03, 72.59}, {"jaipur", 26.91, 75.79},
             {"surat", 21.17, 72.83},     {"lucknow", 26.85, 80.95},
             {"kanpur", 26.46, 80.33},    {"nagpur", 21.15, 79.09},
             {"indore", 22.72, 75.86},    {"bhopal", 23.26, 77.41},
             {"patna", 25.59, 85.14},     {"vadodara", 22.31, 73.19},
             {nullptr, 19.08, 72.88}};
  for (int i = 0; lut[i].k; i++) {
    if (c.indexOf(lut[i].k) >= 0) {
      lat = lut[i].la;
      lon = lut[i].lo;
      return;
    }
  }
  lat = 19.08;
  lon = 72.88; // default Mumbai
}

String wmoToDesc(int code) {
  if (code == 0)
    return "Clear sky";
  if (code <= 3)
    return "Partly cloudy";
  if (code <= 9)
    return "Foggy";
  if (code <= 19)
    return "Drizzle";
  if (code <= 29)
    return "Rain";
  if (code <= 39)
    return "Snow/Sleet";
  if (code <= 49)
    return "Foggy";
  if (code <= 59)
    return "Drizzle";
  if (code <= 69)
    return "Rain";
  if (code <= 79)
    return "Snow";
  if (code <= 84)
    return "Showers";
  if (code <= 99)
    return "Thunderstorm";
  return "Unknown";
}

// ═══════════════════════════════════════════════
//  LITTLEFS HELPERS
// ═══════════════════════════════════════════════
bool loadSysConfig() {
  if (!LittleFS.exists(PATH_CONFIG))
    return false;
  File f = LittleFS.open(PATH_CONFIG, "r");
  if (!f)
    return false;
  JsonDocument doc;
  if (deserializeJson(doc, f)) {
    f.close();
    return false;
  }
  sysCfg.wifiSSID = doc["wifiSSID"] | "";
  sysCfg.wifiPass = doc["wifiPass"] | "";
  sysCfg.ownerName = doc["ownerName"] | "";
  sysCfg.location = doc["location"] | "";
  sysCfg.lat = doc["lat"] | 19.08f;
  sysCfg.lon = doc["lon"] | 72.88f;
  f.close();
  return sysCfg.wifiSSID.length() > 0;
}

void saveSysConfig() {
  File f = LittleFS.open(PATH_CONFIG, "w");
  JsonDocument doc;
  doc["wifiSSID"] = sysCfg.wifiSSID;
  doc["wifiPass"] = sysCfg.wifiPass;
  doc["ownerName"] = sysCfg.ownerName;
  doc["location"] = sysCfg.location;
  doc["lat"] = sysCfg.lat;
  doc["lon"] = sysCfg.lon;
  serializeJson(doc, f);
  f.close();
}

void loadProfiles() {
  profileCount = 0;
  if (!LittleFS.exists(PATH_PROFILES))
    return;
  File f = LittleFS.open(PATH_PROFILES, "r");
  if (!f)
    return;
  JsonDocument doc;
  if (deserializeJson(doc, f)) {
    f.close();
    return;
  }
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject o : arr) {
    if (profileCount >= MAX_PROFILES)
      break;
    profiles[profileCount++] = {o["uid"] | "",
                                o["name"] | "",
                                o["role"] | "family",
                                o["age"] | "",
                                o["gender"] | "",
                                o["note"] | "",
                                0,
                                ""};
  }
  f.close();
}

void saveProfiles() {
  File f = LittleFS.open(PATH_PROFILES, "w");
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < profileCount; i++) {
    JsonObject o = arr.add<JsonObject>();
    o["uid"] = profiles[i].uid;
    o["name"] = profiles[i].name;
    o["role"] = profiles[i].role;
    o["age"] = profiles[i].age;
    o["gender"] = profiles[i].gender;
    o["note"] = profiles[i].note;
  }
  serializeJson(doc, f);
  f.close();
}

void loadTasks() {
  taskCount = 0;
  if (!LittleFS.exists(PATH_TASKS))
    return;
  File f = LittleFS.open(PATH_TASKS, "r");
  if (!f)
    return;
  JsonDocument doc;
  if (deserializeJson(doc, f)) {
    f.close();
    return;
  }
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject o : arr) {
    if (taskCount >= MAX_TASKS)
      break;
    tasks[taskCount++] = {o["id"] | "",
                          o["owner"] | "all",
                          o["text"] | "",
                          o["dueTime"] | "",
                          o["dueDate"] | "",
                          (bool)(o["done"] | false),
                          (bool)(o["reminder"] | false)};
  }
  f.close();
}

void saveTasks() {
  File f = LittleFS.open(PATH_TASKS, "w");
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < taskCount; i++) {
    JsonObject o = arr.add<JsonObject>();
    o["id"] = tasks[i].id;
    o["owner"] = tasks[i].owner;
    o["text"] = tasks[i].text;
    o["dueTime"] = tasks[i].dueTime;
    o["dueDate"] = tasks[i].dueDate;
    o["done"] = tasks[i].done;
    o["reminder"] = tasks[i].reminder;
  }
  serializeJson(doc, f);
  f.close();
}

void appendRFIDLog(const String &uid, const String &who) {
  File f = LittleFS.open(PATH_RFID_LOG, "a");
  if (!f)
    return;
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  char buf[22];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
  f.println(String(buf) + " | " + uid + " | " +
            (who.length() ? who : "Unknown"));
  f.close();
}

// ═══════════════════════════════════════════════
//  WIFI
// ═══════════════════════════════════════════════
bool tryWifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(sysCfg.wifiSSID.c_str(), sysCfg.wifiPass.c_str());
  unsigned long s = millis();
  int dot = 0;
  while (millis() - s < WIFI_CONNECT_TIMEOUT) {
    showConnecting(sysCfg.wifiSSID, dot++);
    if (WiFi.status() == WL_CONNECTED)
      return true;
    delay(400);
  }
  return false;
}

void startPortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress ip = WiFi.softAPIP();
  showPortalScreen(ip.toString());
  Serial.print(F("Portal: "));
  Serial.println(ip);
  portalMode = true;
}

void stopPortal() {
  server.stop();
  WiFi.softAPdisconnect(true);
  serverRunning = false;
  portalMode = false;
}

// ═══════════════════════════════════════════════
//  NTP
// ═══════════════════════════════════════════════
void syncNTP() {
  configTime(TZ_OFFSET_SEC, 0, NTP_SERVER1, NTP_SERVER2);
  cls();
  txt(4, 10, 1, "Syncing time...");
  oled();
  unsigned long s = millis();
  while (time(nullptr) < 100000 && millis() - s < 10000)
    delay(200);
  timeReady = time(nullptr) > 100000;
  Serial.println(timeReady ? F("NTP OK") : F("NTP fail"));
}

// ═══════════════════════════════════════════════
//  WEATHER (Open-Meteo — 100% free, no key)
// ═══════════════════════════════════════════════
void fetchWeather() {
  if (!wifiConnected)
    return;
  float lat, lon;
  cityCoords(sysCfg.location, lat, lon);

  WiFiClient wc;
  HTTPClient http;
  // current_weather + hourly for humidity/wind
  String url =
      "http://api.open-meteo.com/v1/forecast?latitude=" + String(lat, 3) +
      "&longitude=" + String(lon, 3) +
      "&current_weather=true&hourly=relativehumidity_2m,apparent_temperature,"
      "windspeed_10m&forecast_days=1";
  http.begin(wc, url);
  http.setTimeout(8000);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String body = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, body)) {
      auto cw = doc["current_weather"];
      weather.temp = cw["temperature"] | 0.0f;
      weather.code = cw["weathercode"] | -1;
      weather.windSpeed = cw["windspeed"] | 0.0f;
      weather.desc = wmoToDesc(weather.code);
      // grab humidity from hourly index 0
      weather.humidity = doc["hourly"]["relativehumidity_2m"][0] | 0;
      weather.feelsLike =
          doc["hourly"]["apparent_temperature"][0] | weather.temp;
      weather.fetchedAt = millis();
      Serial.printf("Weather: %.1f°C %s\n", weather.temp, weather.desc.c_str());
    }
  }
  http.end();

  // AQI from Open-Meteo air quality API
  String aqUrl =
      "http://air-quality-api.open-meteo.com/v1/air-quality?latitude=" +
      String(lat, 3) + "&longitude=" + String(lon, 3) +
      "&hourly=european_aqi&forecast_days=1";
  http.begin(wc, aqUrl);
  http.setTimeout(6000);
  if (http.GET() == HTTP_CODE_OK) {
    String body = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, body)) {
      weather.aqi = doc["hourly"]["european_aqi"][0] | -1;
    }
  }
  http.end();
  lastWeatherMs = millis();
}

// ═══════════════════════════════════════════════
//  MOISTURE SENSOR
// ═══════════════════════════════════════════════
void readMoisture() {
  if (!MOISTURE_ENABLED) {
    moisturePct = -1;
    return;
  }
  int raw = analogRead(MOISTURE_PIN);
  moisturePct = constrain(map(raw, MOISTURE_DRY, MOISTURE_WET, 0, 100), 0, 100);
  lastMoistureMs = millis();
}

// ═══════════════════════════════════════════════
//  RFID — who owns this card?
// ═══════════════════════════════════════════════
String whoIsCard(const String &uid) {
  for (int i = 0; i < profileCount; i++) {
    if (profiles[i].uid.length() && profiles[i].uid.equalsIgnoreCase(uid)) {
      // update last seen
      profiles[i].lastSeen = millis();
      time_t now = time(nullptr);
      struct tm *t = localtime(&now);
      char buf[8];
      strftime(buf, sizeof(buf), "%H:%M", t);
      profiles[i].lastSeenTime = String(buf);
      return profiles[i].name;
    }
  }
  return "";
}

// ═══════════════════════════════════════════════
//  WEB SERVER ROUTES
// ═══════════════════════════════════════════════
void setupRoutes() {

  // ── Setup portal page
  server.on("/", HTTP_GET, []() {
    if (!portalMode) {
      server.send_P(200, "text/html", HTML_DASHBOARD);
    } else {
      server.send_P(200, "text/html", HTML_SETUP);
    }
  });

  // ── Save WiFi + profile from setup page
  server.on("/save", HTTP_POST, []() {
    sysCfg.wifiSSID = server.arg("ssid");
    sysCfg.wifiPass = server.arg("password");
    sysCfg.ownerName = server.arg("owner_name");
    sysCfg.location = server.arg("location");
    cityCoords(sysCfg.location, sysCfg.lat, sysCfg.lon);
    saveSysConfig();

    // Auto-create owner profile if name given
    String oname = server.arg("owner_name");
    if (oname.length() && profileCount < MAX_PROFILES) {
      // check not duplicate
      bool found = false;
      for (int i = 0; i < profileCount; i++)
        if (profiles[i].role == "owner") {
          found = true;
          break;
        }
      if (!found) {
        profiles[profileCount++] = {"",
                                    oname,
                                    "owner",
                                    server.arg("owner_age"),
                                    server.arg("owner_gender"),
                                    "",
                                    0,
                                    ""};
        saveProfiles();
      }
    }

    // Auto-create plant profile if plant name given
    String pname = server.arg("plant_name");
    if (pname.length() && profileCount < MAX_PROFILES) {
      bool found = false;
      for (int i = 0; i < profileCount; i++)
        if (profiles[i].role == "plant") {
          found = true;
          break;
        }
      if (!found) {
        profiles[profileCount++] = {
            "", pname, "plant", "", "", server.arg("plant_note"), 0, ""};
        saveProfiles();
      }
    }

    bool ok = tryWifiConnect();
    if (ok) {
      wifiConnected = true;
      server.send_P(200, "text/html", HTML_OK);
    } else {
      String page = String(HTML_FAIL);
      page.replace("%SSID%", sysCfg.wifiSSID);
      server.send(200, "text/html", page);
    }
  });

  // ── Dashboard app
  server.on("/app", HTTP_GET,
            []() { server.send_P(200, "text/html", HTML_DASHBOARD); });

  // ── API: status JSON
  server.on("/api/status", HTTP_GET, []() {
    JsonDocument doc;
    doc["ip"] = wifiConnected ? WiFi.localIP().toString() : "";
    doc["rssi"] = wifiConnected ? WiFi.RSSI() : 0;
    doc["uptime"] = millis() / 1000;
    doc["heap"] = ESP.getFreeHeap();

    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    char buf[12];
    strftime(buf, sizeof(buf), "%H:%M:%S", t);
    doc["time"] = timeReady ? String(buf) : "--";

    auto w = doc["weather"].to<JsonObject>();
    w["temp"] = weather.temp;
    w["feels"] = weather.feelsLike;
    w["desc"] = weather.desc;
    w["hum"] = weather.humidity;
    w["wind"] = weather.windSpeed;
    w["aqi"] = weather.aqi;

    doc["moisture"] = moisturePct;
    doc["lastUID"] = lastUID;
    doc["lastWho"] = lastWho;
    doc["lastScanTime"] = lastScanTime;

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
  });

  // ── API: tasks
  server.on("/api/tasks", HTTP_GET, []() {
    JsonDocument doc;
    auto arr = doc["tasks"].to<JsonArray>();
    for (int i = 0; i < taskCount; i++) {
      auto o = arr.add<JsonObject>();
      o["text"] = tasks[i].text;
      o["owner"] = tasks[i].owner;
      o["dueTime"] = tasks[i].dueTime;
      o["dueDate"] = tasks[i].dueDate;
      o["done"] = tasks[i].done;
    }
    auto parr = doc["profiles"].to<JsonArray>();
    for (int i = 0; i < profileCount; i++)
      parr.add(profiles[i].name);
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
  });

  server.on("/api/tasks/add", HTTP_POST, []() {
    if (taskCount >= MAX_TASKS) {
      server.send(400, "text/plain", "Full");
      return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    tasks[taskCount++] = {String(millis()),
                          doc["owner"] | "all",
                          doc["text"] | "",
                          doc["dueTime"] | "",
                          doc["dueDate"] | "",
                          false,
                          false};
    saveTasks();
    server.send(200, "text/plain", "ok");
  });

  // Task toggle: POST /api/tasks/toggle  body = plain index
  server.on("/api/tasks/toggle", HTTP_POST, []() {
    int idx = server.arg("plain").toInt();
    if (idx >= 0 && idx < taskCount)
      tasks[idx].done = !tasks[idx].done;
    saveTasks();
    server.send(200, "text/plain", "ok");
  });

  // Task delete: POST /api/tasks/delete  body = plain index
  server.on("/api/tasks/delete", HTTP_POST, []() {
    int idx = server.arg("plain").toInt();
    if (idx >= 0 && idx < taskCount) {
      for (int i = idx; i < taskCount - 1; i++)
        tasks[i] = tasks[i + 1];
      taskCount--;
    }
    saveTasks();
    server.send(200, "text/plain", "ok");
  });

  // ── API: profiles
  server.on("/api/profiles", HTTP_GET, []() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < profileCount; i++) {
      auto o = arr.add<JsonObject>();
      o["name"] = profiles[i].name;
      o["role"] = profiles[i].role;
      o["uid"] = profiles[i].uid;
      o["age"] = profiles[i].age;
      o["gender"] = profiles[i].gender;
      o["note"] = profiles[i].note;
    }
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
  });

  server.on("/api/profiles/add", HTTP_POST, []() {
    if (profileCount >= MAX_PROFILES) {
      server.send(400, "text/plain", "Full");
      return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    profiles[profileCount++] = {doc["uid"] | "",
                                doc["name"] | "Unknown",
                                doc["role"] | "family",
                                doc["age"] | "",
                                doc["gender"] | "",
                                doc["note"] | "",
                                0,
                                ""};
    saveProfiles();
    server.send(200, "text/plain", "ok");
  });

  server.on("/api/profiles/delete", HTTP_POST, []() {
    int idx = server.arg("plain").toInt();
    if (idx >= 0 && idx < profileCount) {
      for (int i = idx; i < profileCount - 1; i++)
        profiles[i] = profiles[i + 1];
      profileCount--;
    }
    saveProfiles();
    server.send(200, "text/plain", "ok");
  });

  // ── API: RFID log
  server.on("/api/log", HTTP_GET, []() {
    if (!LittleFS.exists(PATH_RFID_LOG)) {
      server.send(200, "text/plain", "");
      return;
    }
    File f = LittleFS.open(PATH_RFID_LOG, "r");
    String out = "";
    while (f.available())
      out += f.readStringUntil('\n') + "\n";
    f.close();
    server.send(200, "text/plain", out);
  });

  server.on("/api/log/clear", HTTP_POST, []() {
    LittleFS.remove(PATH_RFID_LOG);
    server.send(200, "text/plain", "ok");
  });

  // ── API: config
  server.on("/api/config", HTTP_GET, []() {
    JsonDocument doc;
    doc["ownerName"] = sysCfg.ownerName;
    doc["location"] = sysCfg.location;
    doc["wifiSSID"] = sysCfg.wifiSSID;
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
  });

  server.on("/api/config/save", HTTP_POST, []() {
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    if (doc["ownerName"].as<String>().length())
      sysCfg.ownerName = doc["ownerName"].as<String>();
    if (doc["location"].as<String>().length())
      sysCfg.location = doc["location"].as<String>();
    if (doc["wifiSSID"].as<String>().length())
      sysCfg.wifiSSID = doc["wifiSSID"].as<String>();
    if (doc["wifiPass"].as<String>().length())
      sysCfg.wifiPass = doc["wifiPass"].as<String>();
    cityCoords(sysCfg.location, sysCfg.lat, sysCfg.lon);
    saveSysConfig();
    server.send(200, "text/plain", "ok");
  });

  // ── Factory reset
  server.on("/reset", HTTP_GET, []() {
    LittleFS.remove(PATH_CONFIG);
    LittleFS.remove(PATH_PROFILES);
    LittleFS.remove(PATH_TASKS);
    LittleFS.remove(PATH_RFID_LOG);
    server.send(200, "text/plain", "Reset. Rebooting...");
    delay(500);
    ESP.restart();
  });

  // ── Redirect anything else to /
  server.onNotFound([]() {
    server.sendHeader("Location", "/");
    server.send(302);
  });
}

// ═══════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\n\n=== NEO BOOT ==="));

  // ── OLED (must be before SPI.begin) ──────────
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("OLED FAILED"));
    while (true)
      delay(100);
  }
  display.cp437(true);
  Serial.println(F("OLED OK"));

  bootAnimation();

  // ── LittleFS ─────────────────────────────────
  if (!LittleFS.begin()) {
    Serial.println(F("LittleFS fail — formatting"));
    LittleFS.format();
    LittleFS.begin();
  }
  loadSysConfig();
  loadProfiles();
  loadTasks();

  // ── RFID (hardware SPI) ──────────────────────
  SPI.begin();
  pinMode(RFID_SS, OUTPUT);
  digitalWrite(RFID_SS, HIGH);
  rfid.PCD_Init();
  delay(50);
  byte ver = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.printf("RFID ver: 0x%02X\n", ver);
  if (ver == 0x00 || ver == 0xFF) {
    cls();
    txt(4, 10, 1, "RFID FAILED!");
    txt(4, 26, 1, "Check wiring");
    oled();
    while (true)
      delay(100);
  }
  Serial.println(F("RFID OK"));

  // ── WiFi ─────────────────────────────────────
  if (sysCfg.wifiSSID.length()) {
    wifiConnected = tryWifiConnect();
  }

  setupRoutes();
  server.begin();
  serverRunning = true;

  if (!wifiConnected) {
    startPortal();
    // Wait for user to configure via portal
    unsigned long pStart = millis();
    while (!wifiConnected && millis() - pStart < PORTAL_TIMEOUT_MS) {
      server.handleClient();
      delay(5);
    }
  }

  if (wifiConnected) {
    if (portalMode)
      stopPortal();
    Serial.println(F("WiFi connected: ") + WiFi.localIP().toString());
    syncNTP();
    readMoisture();
    fetchWeather();
    showWifiSuccess(sysCfg.ownerName, WiFi.localIP().toString());
  } else {
    cls();
    txt(4, 10, 1, "No WiFi.");
    txt(4, 24, 1, "RFID-only mode.");
    oled();
    delay(2000);
  }

  lastPageMs = millis();
  showDashboard();
}

// ═══════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════
void loop() {
  // Web server
  if (serverRunning)
    server.handleClient();

  // If portal just got a WiFi connection
  if (portalMode && wifiConnected) {
    stopPortal();
    syncNTP();
    readMoisture();
    fetchWeather();
    showWifiSuccess(sysCfg.ownerName, WiFi.localIP().toString());
    lastPageMs = millis();
    showDashboard();
    return;
  }

  // Periodic weather
  if (wifiConnected && millis() - lastWeatherMs > WEATHER_INTERVAL_MS)
    fetchWeather();

  // Periodic moisture
  if (MOISTURE_ENABLED && millis() - lastMoistureMs > MOISTURE_INTERVAL_MS)
    readMoisture();

  // Dashboard page rotation
  if (millis() - lastPageMs > DASH_PAGE_MS) {
    lastPageMs = millis();
    nextDashPage();
  }

  // RFID scan
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10)
        uid += "0";
      uid += String(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1)
        uid += ":";
    }
    uid.toUpperCase();
    lastUID = uid;
    lastWho = whoIsCard(uid);

    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    char buf[12];
    strftime(buf, sizeof(buf), "%H:%M:%S", t);
    lastScanTime = timeReady ? String(buf) : "--";
    lastScanMs = millis();

    Serial.println("SCAN | " + uid + " | " +
                   (lastWho.length() ? lastWho : "Unknown"));
    appendRFIDLog(uid, lastWho);
    animateScan(uid, lastWho);

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    lastPageMs = millis();
    showDashboard();
  }
}