#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// MoRa-LC UltraLight v3 Configuration
// Complete Rewrite - Clean Architecture
// ============================================================

// System
#define DEVICE_NAME "MoRa-LC UltraLight"
#define VERSION "3.0.0"

// Display Configuration
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 1  // Landscape mode

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

// BLE Scanner Configuration
// BLE Library Selection:
// - BLE_USE_ESP32: ESP32 BLE Arduino (standard, filters duplicates)
// - BLE_USE_ESP_IDF: ESP-IDF Native API (lightweight, direct control)
// - (neither): NimBLE-Arduino (best duplicate handling, recommended)
// #define BLE_USE_ESP32   // ESP32 BLE Arduino Library
// #define BLE_USE_ESP_IDF  // ESP-IDF Native API (no Arduino wrapper)

// Debug Mode: Serial Only (no Display/UI)
// #define SERIAL_ONLY_MODE  // Comment out to enable Display/UI
#define BLE_SCAN_INTERVAL 100
#define BLE_SCAN_WINDOW   99
#define BLE_RSSI_THRESHOLD -100
#define BEACON_TIMEOUT    15000  // ms - Beacon timeout (15s during race, no cleanup)
#define BLE_UUID_PREFIX "c3:00:"

// Lap Detection
#define DEFAULT_LAP_RSSI_NEAR -65
#define DEFAULT_LAP_RSSI_FAR  -80
#define MIN_RSSI -100
#define MAX_RSSI -30

// Touch
#define TOUCH_DEBOUNCE    200
#define TOUCH_CALIBRATION_MODE  0

// Touch Calibration Data
#define TOUCH_CAL_DATA_ENABLED  1
#if TOUCH_CAL_DATA_ENABLED
static const uint16_t TOUCH_CAL_DATA[8] = { 3754, 241, 3757, 3700, 448, 257, 423, 3663 };
#else
static const uint16_t* TOUCH_CAL_DATA = nullptr;
#endif

// Race Settings
#define MIN_LAP_TIME      10000
#define MAX_TEAMS         20
#define MAX_RACE_DURATION 7200000  // 2 hours max

// Performance
#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)  // 16ms per frame

// Debug/Test Modes
// #define BLE_TEST_MODE  // Uncomment to start with BLE Test Screen

// UI Spacing (8px grid system)
#define GRID_UNIT 8
#define SPACING_SMALL (GRID_UNIT)      // 8px
#define SPACING_MEDIUM (GRID_UNIT * 2) // 16px
#define SPACING_LARGE (GRID_UNIT * 3)   // 24px

// Component Sizes
#define BUTTON_HEIGHT     45
#define BUTTON_MARGIN     8
#define LIST_ITEM_HEIGHT  40
#define HEADER_HEIGHT     35
#define ICON_SIZE_SMALL   16
#define ICON_SIZE_MEDIUM  24
#define ICON_SIZE_LARGE   32
#define ICON_SIZE_XLARGE  48

// Typography
#define TEXT_SIZE_TINY    1
#define TEXT_SIZE_SMALL   1
#define TEXT_SIZE_NORMAL  2
#define TEXT_SIZE_LARGE   3
#define TEXT_SIZE_XLARGE  4


// #define BLE_USE_ESP32  // Comment out to use NimBLE instead (NimBLE has better duplicate handling)
#endif // CONFIG_H

