#pragma once

// ═══════════════════════════════════════════════════════════
//  NEO — Here for You | config.h
//  All pin definitions, constants, and shared structs
// ═══════════════════════════════════════════════════════════

// ── OLED (Software SPI — pins away from HW SPI) ─────────────
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI D3
#define OLED_CLK D4
#define OLED_DC D2
#define OLED_CS D1
#define OLED_RST -1 // RST tied to 3.3V physically

// ── RFID (Hardware SPI) ──────────────────────────────────────
#define RFID_SS D8
#define RFID_RST D0
// HW SPI fixed pins: MOSI=D7, MISO=D6, SCK=D5

// ── Plant moisture sensor ─────────────────────────────────────
#define MOISTURE_PIN A0
#define MOISTURE_ENABLED true // set false if sensor not wired
#define MOISTURE_DRY 800      // ADC value when completely dry
#define MOISTURE_WET 300      // ADC value when fully wet

// ── Timing ───────────────────────────────────────────────────
#define DASH_PAGE_MS 4000          // ms per dashboard page
#define WEATHER_INTERVAL_MS 600000 // 10 min weather refresh
#define MOISTURE_INTERVAL_MS 30000 // 30s moisture check
#define WIFI_CONNECT_TIMEOUT 15000 // 15s connection timeout
#define PORTAL_TIMEOUT_MS 300000   // 5 min portal auto-close

// ── Filesystem paths ─────────────────────────────────────────
#define PATH_CONFIG "/config.json"
#define PATH_PROFILES "/profiles.json"
#define PATH_TASKS "/tasks.json"
#define PATH_RFID_LOG "/rfid.log"

// ── AP portal ────────────────────────────────────────────────
#define AP_SSID "NEO-Setup"
#define AP_PASS "" // open network

// ── NTP (IST = UTC+5:30) ─────────────────────────────────────
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define TZ_OFFSET_SEC (5 * 3600 + 1800)

// ── Max counts ───────────────────────────────────────────────
#define MAX_PROFILES 6
#define MAX_TASKS 20

// ── Structs ──────────────────────────────────────────────────
struct Profile {
  String uid; // RFID UID, empty = unassigned
  String name;
  String role; // "owner","family","partner","plant"
  String age;
  String gender;
  String note;
  unsigned long lastSeen; // millis
  String lastSeenTime;    // formatted
};

struct Task {
  String id;
  String owner; // profile name or "all"
  String text;
  String dueTime; // "HH:MM" or ""
  String dueDate; // "YYYY-MM-DD" or ""
  bool done;
  bool reminder;
};

struct SysConfig {
  String wifiSSID;
  String wifiPass;
  String ownerName;
  String location; // city for weather
  float lat;
  float lon;
};

struct WeatherData {
  float temp;
  float feelsLike;
  int code;
  String desc;
  String icon; // 1-char glyph hint
  int humidity;
  float windSpeed;
  int aqi; // -1 = not fetched
  unsigned long fetchedAt;
};