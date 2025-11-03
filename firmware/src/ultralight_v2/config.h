#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// MoRa-LC UltraLight v2 Configuration (LovyanGFX)
// ============================================================

// System
#define DEVICE_NAME "MoRa-LC UltraLight"
#define VERSION "2.0.0"

// Display
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 230
#define BACKGROUND_COLOR 0xCE59  // Light Gray (heller Hintergrund für bessere Lesbarkeit)
#define TEXT_COLOR 0x0000        // Black (dunkler Text auf hellem Hintergrund)
#define ACCENT_COLOR 0x07E0      // Green

// Pin Definitions (ESP32-2432S028 - CheapYellow Display)
// TFT Display (ST7789)
#define TFT_MOSI_PIN      13
#define TFT_MISO_PIN      12
#define TFT_SCLK_PIN      14
#define TFT_CS_PIN        15
#define TFT_DC_PIN        2
#define TFT_RST_PIN       -1  // Not used
#define TFT_BL_PIN        21  // Backlight

// XPT2046 Touch Controller (Resistive)
#define TOUCH_CS_PIN      33
#define TOUCH_IRQ_PIN     36   // Optional
#define TOUCH_MOSI_PIN    32   // Separate SPI bus for Touch
#define TOUCH_MISO_PIN    39
#define TOUCH_CLK_PIN     25

// SD Card (SPI) - VSPI standard pins
#define SD_CS_PIN         5
#define SD_MOSI_PIN       23
#define SD_MISO_PIN       19
#define SD_SCLK_PIN       18
#define SD_SPI_FREQ       4000000  // 4 MHz

// BLE Scanner
#define BLE_SCAN_INTERVAL 100
#define BLE_SCAN_WINDOW   99
#define BLE_RSSI_THRESHOLD -100
#define BEACON_TIMEOUT    15000  // ms - Beacon gilt als "verschwunden" nach 15s (optimiert)
#define BLE_SCAN_DURATION 10     // seconds - Scan-Duration (länger = stabiler)
#define BLE_UUID_PREFIX "c3:00:"

// Lap Detection
#define DEFAULT_LAP_RSSI_NEAR -65
#define DEFAULT_LAP_RSSI_FAR  -80
#define MIN_RSSI -100
#define MAX_RSSI -30

// Touch
#define TOUCH_DEBOUNCE    200
#define TOUCH_CALIBRATION_MODE  0  // Set to 1 to enable calibration on startup

// Touch Calibration Data (from calibrateTouch() - update after calibration)
// Format: { x_min, y_min, x_min, y_max, x_max, y_min, x_max, y_max }
// Calibrated values (update this after running calibration):
#define TOUCH_CAL_DATA_ENABLED  1  // Set to 1 to use calibration data
#if TOUCH_CAL_DATA_ENABLED
static const uint16_t TOUCH_CAL_DATA[8] = { 3754, 241, 3757, 3700, 448, 257, 423, 3663 };
#else
static const uint16_t* TOUCH_CAL_DATA = nullptr;
#endif

// Race Settings
#define MIN_LAP_TIME      10000
#define MAX_TEAMS         20
#define MAX_RACE_DURATION 7200000

// UI - Größere Elemente für bessere Bedienbarkeit
#define BUTTON_HEIGHT     45        // Größer: 42 -> 50
#define BUTTON_MARGIN     8        // Mehr Abstand: 8 -> 10
#define LIST_ITEM_HEIGHT  40        // Kompakter: 48 -> 40 (mehr Teams sichtbar!)
#define HEADER_HEIGHT     35        // Größer: 30 -> 40
#define TEXT_SIZE_NORMAL  2         // Standard Text-Größe
#define TEXT_SIZE_LARGE   3         // Große Überschriften

// Colors (RGB565) - Helles Design für bessere Lesbarkeit draußen
#define COLOR_PRIMARY     0x001F     // Dark Blue (dunkler auf hellem Hintergrund)
#define COLOR_SECONDARY   0x07E0     // Green
#define COLOR_DANGER      0xF800     // Red
#define COLOR_WARNING     0xFD20     // Orange
#define COLOR_BUTTON      0x8410     // Medium Gray (heller als vorher)
#define COLOR_BUTTON_TEXT 0x0000     // Black text on buttons
#define COLOR_BUTTON_PRESSED 0x630C  // Darker Gray when pressed
#define COLOR_HEADER_BG   0x001F     // Dark Blue header
#define COLOR_HEADER_TEXT 0xFFFF     // White text in header

#endif // CONFIG_H

