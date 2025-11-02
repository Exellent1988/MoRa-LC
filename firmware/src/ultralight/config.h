#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// MoRa-LC UltraLight Configuration
// ============================================================

// System
#define DEVICE_NAME "MoRa-LC UltraLight"
#define VERSION "0.1.0"

// Display
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BACKGROUND_COLOR TFT_BLACK
#define TEXT_COLOR TFT_WHITE
#define ACCENT_COLOR TFT_GREEN

// Pin Definitions (ESP32-2432S028 - CheapYellow Display)
// TFT Display (ST7789) - defined in platformio.ini
// TFT_MOSI=13, TFT_SCLK=14, TFT_CS=15, TFT_DC=2, TFT_RST=-1, TFT_BL=21

// XPT2046 Touch Controller (Resistive, shared SPI with TFT)
#ifndef XPT2046_CS
#define XPT2046_CS       33
#endif
#ifndef XPT2046_IRQ
#define XPT2046_IRQ      36  // Optional, can be 255 to disable
#endif
#ifndef XPT2046_MOSI
#define XPT2046_MOSI     13  // Shared with TFT
#endif
#ifndef XPT2046_MISO
#define XPT2046_MISO     12  // Important! Different from TFT
#endif
#ifndef XPT2046_CLK
#define XPT2046_CLK      14  // Shared with TFT
#endif

// SD Card (SPI)
#define SD_CS_PIN        5

// BLE Scanner
#define BLE_SCAN_INTERVAL 100      // ms
#define BLE_SCAN_WINDOW 99         // ms
#define BLE_RSSI_THRESHOLD -80     // dBm
#define BLE_PROXIMITY_THRESHOLD 10.0 // meters (sehr tolerant für Testing)
#define BEACON_TIMEOUT 30000       // ms (30 seconds - viel toleranter)

// Touch
#define TOUCH_DEBOUNCE 200         // ms

// Race Settings
#define MIN_LAP_TIME 10000         // ms (10 seconds)
#define MAX_TEAMS 20
#define MAX_RACE_DURATION 7200000  // ms (2 hours)

// UI
#define BUTTON_HEIGHT 50
#define BUTTON_MARGIN 10
#define LIST_ITEM_HEIGHT 40
#define HEADER_HEIGHT 30

// Colors
#define COLOR_PRIMARY 0x0014     // Dark Blue
#define COLOR_SECONDARY 0x07E0   // Green
#define COLOR_DANGER 0xF800      // Red
#define COLOR_WARNING 0xFD20     // Orange
#define COLOR_BUTTON 0x5AEB      // Light Gray
#define COLOR_BUTTON_PRESSED 0x3186  // Darker Gray

#endif // CONFIG_H


