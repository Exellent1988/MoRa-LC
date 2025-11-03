#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "display_manager.h"
#include "ui_state.h"
#include "ui_screens.h"
#include "../../lib/BLEScanner/BLEScanner.h"
#include "../../lib/LapCounter/LapCounter.h"
#include "../../lib/DataLogger/DataLogger.h"
#include "../ultralight/persistence.h"
#include <map>

// Disable ESP-IDF logging for NimBLE completely
#include "esp_log.h"

// ============================================================
// MoRa-LC UltraLight v2 (LovyanGFX)
// ============================================================

// Global instances
DisplayManager display;
BLEScanner bleScanner;
LapCounter lapCounter;
DataLogger dataLogger;
PersistenceManager persistence;

// Global state
bool raceRunning = false;
uint32_t raceStartTime = 0;
uint32_t raceDuration = 60000;  // 1 minute default (will be loaded from persistence)
String currentRaceName = "Test Race";
int8_t lapRssiNear = DEFAULT_LAP_RSSI_NEAR;
int8_t lapRssiFar = DEFAULT_LAP_RSSI_FAR;
std::map<uint8_t, bool> beaconPresence;

// BLE Callback for lap detection
void onBeaconDetected(const BeaconData& beacon) {
    // Only count laps during race
    if (!raceRunning) {
        return;
    }
    
    // Check if beacon belongs to a team (by MAC address)
    TeamData* team = lapCounter.getTeamByBeacon(beacon.macAddress);
    
    if (!team) {
        // Unknown beacon - silent
        return;
    }
    
    // RSSI-based presence detection with hysteresis
    if (beacon.rssi > lapRssiNear) {
        // Beacon is NEAR (strong signal)
        if (!beaconPresence[team->teamId]) {
            // Was away, now near → count lap!
            if (lapCounter.recordLap(team->teamId, beacon.lastSeen)) {
                Serial.printf("[Lap] ✅ Team %u (%s): Lap %u\n",
                             team->teamId, team->teamName.c_str(), team->lapCount);
                
                // Log to SD
                if (dataLogger.isReady() && !team->laps.empty()) {
                    LapTime& lap = team->laps.back();
                    dataLogger.logLap(team->teamId, team->teamName, 
                                     lap.lapNumber, lap.timestamp, lap.duration);
                }
                
                // Update screen
                uiState.needsRedraw = true;
            }
            
            beaconPresence[team->teamId] = true;
        }
    } else if (beacon.rssi < lapRssiFar) {
        // Beacon is AWAY (weak signal)
        if (beaconPresence[team->teamId]) {
            beaconPresence[team->teamId] = false;
        }
    }
}

// ============================================================
// Initialization
// ============================================================

void initSD() {
    Serial.println("[SD] Initializing...");
    
    // CRITICAL: Initialize SD BEFORE Touch!
    // SD uses VSPI pins (23, 19, 18) - Touch uses HSPI (32, 39, 25)
    // But SD.begin() must be called before any other SPI initialization
    // DataLogger::begin() only takes CS pin - it uses SD library default SPI config
    if (!dataLogger.begin(SD_CS_PIN)) {
        Serial.println("[SD] WARNING: SD card not available");
        return;
    }
    
    Serial.println("[SD] Initialized successfully");
}

void initBLE() {
    Serial.println("[BLE] Initializing...");
    
    // Log levels already set in setup() before any BLE init
    if (!bleScanner.begin()) {
        Serial.println("[BLE] ERROR: Failed to initialize");
        return;
    }
    
    bleScanner.setRSSIThreshold(BLE_RSSI_THRESHOLD);
    bleScanner.setUUIDFilter(BLE_UUID_PREFIX);
    
    Serial.println("[BLE] Initialized successfully");
}

void initPersistence() {
    Serial.println("[Persistence] Initializing...");
    
    if (!persistence.begin()) {
        Serial.println("[Persistence] ERROR: Failed to initialize");
        return;
    }
    
    // Load teams from NVS
    persistence.loadTeams(lapCounter);
    
    // Load race config (duration is in minutes)
    uint32_t raceDurationMinutes = 60;  // Default 60 minutes
    persistence.loadConfig(currentRaceName, raceDurationMinutes);
    raceDuration = raceDurationMinutes * 60 * 1000;  // Convert minutes to milliseconds
    
    // Load RSSI thresholds
    int8_t rssiNear = DEFAULT_LAP_RSSI_NEAR;
    int8_t rssiFar = DEFAULT_LAP_RSSI_FAR;
    persistence.loadRssiThresholds(rssiNear, rssiFar);
    lapRssiNear = rssiNear;
    lapRssiFar = rssiFar;
    
    Serial.printf("[Persistence] Loaded %u teams\n", lapCounter.getTeamCount());
}

// ============================================================
// Setup & Loop
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // CRITICAL: Disable ESP-IDF logging completely BEFORE anything else
    // This must happen BEFORE NimBLE initialization to prevent log spam
    esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
    esp_log_level_set("NimBLE", ESP_LOG_NONE);
    esp_log_level_set("BLE", ESP_LOG_NONE);
    esp_log_level_set("BT", ESP_LOG_NONE);
    esp_log_level_set("BTDM", ESP_LOG_NONE);
    esp_log_level_set("BT_HCI", ESP_LOG_NONE);
    
    Serial.println("\n=================================");
    Serial.println(DEVICE_NAME " v" VERSION);
    Serial.println("=================================\n");
    
    // CRITICAL ORDER:
    // 1. SD Card first (uses VSPI, must be initialized before other SPI)
    initSD();
    
    // 2. Display/Touch (uses separate SPI buses - LovyanGFX handles this)
    Serial.println("[Init] Initializing Display...");
    if (!display.begin()) {
        Serial.println("[Init] ERROR: Display initialization failed!");
        while(1) delay(1000);
    }
    
    // Show initialization screen
    display.fillScreen(BACKGROUND_COLOR);
    display.setTextColor(TEXT_COLOR);
    display.setTextSize(TEXT_SIZE_LARGE);
    display.setCursor(10, 10);
    display.println("MoRa-LC UltraLight");
    display.setTextSize(TEXT_SIZE_NORMAL);
    display.setCursor(10, 50);
    display.println("v" VERSION);
    
    // Show SD status
    display.setCursor(10, 80);
    if (dataLogger.isReady()) {
        display.setTextColor(COLOR_SECONDARY);
        display.println("SD: OK");
    } else {
        display.setTextColor(COLOR_WARNING);
        display.println("SD: No Card");
    }
    display.setTextColor(TEXT_COLOR);
    
    // 3. BLE Scanner
    initBLE();
    bleScanner.onBeaconDetected(onBeaconDetected);  // Set callback for lap detection
    display.setCursor(10, 100);
    display.println("BLE: OK");
    
    // 4. Persistence
    initPersistence();
    display.setCursor(10, 120);
    display.println("Persistence: OK");
    
    Serial.println("\nSetup complete!");
    delay(2000);
    
    // Touch calibration (if enabled)
    #if TOUCH_CALIBRATION_MODE
    Serial.println("\n[Init] Starting touch calibration...");
    display.calibrateTouch();
    #endif
    
    // Initialize UI state and show home screen
    uiState.changeScreen(SCREEN_HOME);
}

// Forward declarations
void drawScreen();
void drawTeamBeaconAssignScreen();
void drawBeaconListScreen();

void loop() {
    // DEBUG: Print status every 5 seconds
    static uint32_t lastDebug = 0;
    if (millis() - lastDebug > 5000) {
        Serial.printf("[DEBUG] Screen=%d, Scanning=%d, Beacons=%d\n", 
                     uiState.currentScreen, bleScanner.isScanning(), bleScanner.getBeacons().size());
        lastDebug = millis();
    }
    
    // Touch handling
    uint16_t x, y;
    if (display.getTouch(&x, &y)) {
        // Debounce
        if (millis() - uiState.lastTouchTime < TOUCH_DEBOUNCE) {
            delay(10);
            return;
        }
        uiState.lastTouchTime = millis();
        
        Serial.printf("[Touch] x=%u, y=%u, Screen=%d\n", x, y, uiState.currentScreen);
        
        // Debug: Show which buttons would be hit on HOME screen
        if (uiState.currentScreen == SCREEN_HOME) {
            int btnHeight = 42;
            int btnSpacing = 8;
            int btnY = HEADER_HEIGHT + 10;
            int btnWidth = SCREEN_WIDTH - 2 * BUTTON_MARGIN;
            Serial.printf("  -> Button Y ranges: B1=%d-%d, B2=%d-%d, B3=%d-%d, B4=%d-%d\n",
                         btnY, btnY+btnHeight,
                         btnY+btnHeight+btnSpacing, btnY+2*btnHeight+btnSpacing,
                         btnY+2*(btnHeight+btnSpacing), btnY+3*btnHeight+2*btnSpacing,
                         btnY+3*(btnHeight+btnSpacing), btnY+4*btnHeight+3*btnSpacing);
            Serial.printf("  -> Touch Y=%u is ", y);
            if (y >= btnY && y < btnY + btnHeight) Serial.println("in Button 1 (Neues Rennen)");
            else if (y >= btnY + btnHeight + btnSpacing && y < btnY + 2*btnHeight + btnSpacing) Serial.println("in Button 2 (Teams)");
            else if (y >= btnY + 2*(btnHeight+btnSpacing) && y < btnY + 3*btnHeight + 2*btnSpacing) Serial.println("in Button 3 (Ergebnisse)");
            else if (y >= btnY + 3*(btnHeight+btnSpacing) && y < btnY + 4*btnHeight + 3*btnSpacing) Serial.println("in Button 4 (Einstellungen)");
            else Serial.println("NOT in any button!");
        }
        
        handleTouch(x, y);
    }
    
    // Screen redraw if needed
    if (uiState.needsRedraw) {
        drawScreen();
        uiState.needsRedraw = false;
    }
    
    // BLE scanning - start continuous scan if not already scanning (only when needed)
    // CRITICAL: Only start scanning when actually needed (beacon assignment or race running)
    // NOT during race setup - this was causing the system to freeze
    if (!raceRunning && !bleScanner.isScanning()) {
        // Only scan when in beacon assignment screens (not race setup!)
        if (uiState.currentScreen == SCREEN_TEAM_BEACON_ASSIGN || 
            uiState.currentScreen == SCREEN_BEACON_LIST) {
            Serial.printf("[BLE] Auto-starting scan (screen=%d)\n", uiState.currentScreen);
            bleScanner.startScan(0);  // 0 = continuous
        }
    } else if (raceRunning && !bleScanner.isScanning()) {
        Serial.println("[BLE] Auto-starting scan (race running)");
        bleScanner.startScan(0);
    }
    
    // Stop scanning when leaving beacon assignment screens
    if (bleScanner.isScanning() && !raceRunning) {
        if (uiState.currentScreen != SCREEN_TEAM_BEACON_ASSIGN && 
            uiState.currentScreen != SCREEN_BEACON_LIST) {
            bleScanner.stopScan();
        }
    }
    
    // NOTE: Log levels are set once in setup() and should NOT be changed during runtime
    // Calling esp_log_level_set() here causes blocking and UI freezes!
    
    // Auto-refresh beacon screens while scanning (from old variant - prevents freezing)
    // CRITICAL: Must call screen draw functions directly, not just set needsRedraw
    // This prevents the UI from freezing during beacon scanning
    if (bleScanner.isScanning()) {
        Serial.println("[DEBUG] BLE is scanning...");
        if (uiState.currentScreen == SCREEN_TEAM_BEACON_ASSIGN || 
            uiState.currentScreen == SCREEN_BEACON_LIST) {
            Serial.println("[DEBUG] On beacon screen, checking refresh timer...");
            static uint32_t lastBeaconRefresh = 0;
            if (millis() - lastBeaconRefresh > 1000) {  // Refresh every second
                Serial.printf("[AutoRefresh] Updating screen=%d, beacons=%d\n", 
                             uiState.currentScreen, bleScanner.getBeacons().size());
                
                // Direct screen redraw - EXACTLY like old variant
                if (uiState.currentScreen == SCREEN_TEAM_BEACON_ASSIGN) {
                    Serial.println("[AutoRefresh] Drawing TEAM_BEACON_ASSIGN...");
                    drawTeamBeaconAssignScreen();
                    Serial.println("[AutoRefresh] Done!");
                } else if (uiState.currentScreen == SCREEN_BEACON_LIST) {
                    Serial.println("[AutoRefresh] Drawing BEACON_LIST...");
                    drawBeaconListScreen();
                    Serial.println("[AutoRefresh] Done!");
                }
                lastBeaconRefresh = millis();
            } else {
                Serial.printf("[DEBUG] Waiting for refresh... (elapsed=%lu)\n", millis() - lastBeaconRefresh);
            }
        } else {
            Serial.printf("[DEBUG] Not on beacon screen (screen=%d)\n", uiState.currentScreen);
        }
    } else {
        Serial.println("[DEBUG] BLE is NOT scanning");
    }
    
    // Clean up old beacons (only every 5 seconds to reduce overhead)
    static uint32_t lastBeaconCleanup = 0;
    if (millis() - lastBeaconCleanup > 5000) {
        if (bleScanner.isScanning() && !raceRunning) {
            bleScanner.clearOldBeacons(BEACON_TIMEOUT);
        }
        lastBeaconCleanup = millis();
    }
    
    // Update race running screen if active
    if (raceRunning && uiState.currentScreen == SCREEN_RACE_RUNNING) {
        static uint32_t lastRaceUpdate = 0;
        if (millis() - lastRaceUpdate > 500) {  // Update every 500ms
            drawRaceRunningScreen();
            lastRaceUpdate = millis();
        }
    }
    
    delay(10);
}

void drawScreen() {
    switch (uiState.currentScreen) {
        case SCREEN_HOME:
            drawHomeScreen();
            break;
        case SCREEN_TEAMS:
            drawTeamsScreen();
            break;
        case SCREEN_TEAM_EDIT:
            drawTeamEditScreen();
            break;
        case SCREEN_TEAM_BEACON_ASSIGN:
            drawTeamBeaconAssignScreen();
            break;
        case SCREEN_BEACON_LIST:
            drawBeaconListScreen();
            break;
        case SCREEN_RACE_SETUP:
            drawRaceSetupScreen();
            break;
        case SCREEN_RACE_RUNNING:
            drawRaceRunningScreen();
            break;
        case SCREEN_RACE_PAUSED:
            drawRacePausedScreen();
            break;
        case SCREEN_RACE_RESULTS:
            drawRaceResultsScreen();
            break;
        case SCREEN_SETTINGS:
            drawSettingsScreen();
            break;
    }
}
