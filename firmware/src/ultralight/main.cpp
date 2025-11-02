#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "config.h"
#include "BLEScanner.h"
#include "LapCounter.h"
#include "DataLogger.h"
#include "persistence.h"
#include "ui_screens.h"

#ifdef USE_GT911_TOUCH
#include "touch_gt911.h"
TouchGT911 touchGT911;
#elif defined(USE_XPT2046_TOUCH)
#include "touch_xpt2046.h"
TouchXPT2046 touchXPT2046;
#else
#include "touch_calibration.h"
#endif

#include "touch_wrapper.h"

// ============================================================
// MoRa-LC UltraLight
// Standalone Lap Counter mit CheapYellow Display
// ============================================================

// Display
TFT_eSPI tft = TFT_eSPI();

// Core Components
BLEScanner bleScanner;
LapCounter lapCounter;
DataLogger dataLogger;
PersistenceManager persistence;

// Race State
bool raceRunning = false;
uint32_t raceStartTime = 0;
uint32_t raceDuration = 60 * 60 * 1000;  // 60 minutes default
String currentRaceName = "";

// ============================================================
// Forward Declarations
// ============================================================

void initDisplay();
void initBLE();
void initSD();
void initPersistence();
void processTouch();
void drawScreen();
void onBeaconDetected(const BeaconData& beacon);

// ============================================================
// Setup
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=================================");
    Serial.println(DEVICE_NAME " v" VERSION);
    Serial.println("=================================\n");
    
    // Initialize Display
    initDisplay();
    
    tft.fillScreen(BACKGROUND_COLOR);
    tft.setTextColor(TEXT_COLOR);
    tft.setTextSize(2);
    tft.setCursor(10, 100);
    tft.println("Initializing...");
    
    // Initialize BLE Scanner
    initBLE();
    tft.setCursor(10, 130);
    tft.println("BLE: OK");
    
    // Set more lenient RSSI threshold for testing
    bleScanner.setRSSIThreshold(-100);  // Very lenient for testing
    
    // Initialize SD Card
    initSD();
    tft.setCursor(10, 160);
    tft.println("SD: OK");
    
    // Initialize Persistence & Load Teams
    initPersistence();
    tft.setCursor(10, 190);
    tft.println("Loading Teams...");
    
    delay(2000);
    
    Serial.println("\nSetup complete!");
    Serial.println("Ready to scan for beacons.\n");
    
    uiState.needsRedraw = true;
}

// ============================================================
// Main Loop
// ============================================================

void loop() {
    // Handle Touch Input
    processTouch();
    
    // Update Screen if needed
    if (uiState.needsRedraw) {
        drawScreen();
        uiState.needsRedraw = false;
    }
    
    // Auto-refresh beacon screens while scanning
    if (bleScanner.isScanning()) {
        if (uiState.currentScreen == SCREEN_TEAM_BEACON_ASSIGN || 
            uiState.currentScreen == SCREEN_BEACON_LIST) {
            static uint32_t lastBeaconRefresh = 0;
            if (millis() - lastBeaconRefresh > 1000) {  // Refresh every second
                drawScreen();
                lastBeaconRefresh = millis();
            }
        }
    }
    
    // Race Running: Update display
    if (raceRunning) {
        // Update every second
        static uint32_t lastUpdate = 0;
        if (millis() - lastUpdate > 1000) {
            drawRaceRunningScreen();
            lastUpdate = millis();
        }
        
        // Check if race time is up
        if (millis() - raceStartTime >= raceDuration) {
            raceRunning = false;
            uiState.currentScreen = SCREEN_RACE_RESULTS;
            uiState.needsRedraw = true;
            
            bleScanner.stopScan();
            
            Serial.println("\n=== RACE FINISHED ===");
        }
    }
    
    // Clean up old beacons (only when scanning)
    if (bleScanner.isScanning()) {
        static uint32_t lastCleanup = 0;
        if (millis() - lastCleanup > 5000) {  // Check every 5 seconds
            bleScanner.clearOldBeacons(BEACON_TIMEOUT);  // But remove only > 30s old
            lastCleanup = millis();
        }
    }
    
    delay(10);
}

// ============================================================
// Initialization
// ============================================================

void initDisplay() {
    Serial.println("[Display] Initializing...");
    
    tft.init();
    tft.setRotation(1);  // Landscape
    tft.fillScreen(BACKGROUND_COLOR);
    
    // Backlight on
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    // Touch initialization based on display version
    #ifdef USE_GT911_TOUCH
        // GT911 Capacitive Touch
        if (touchGT911.begin()) {
            Serial.println("[Display] Initialized with GT911 Touch");
        } else {
            Serial.println("[Display] GT911 Touch init failed!");
        }
    #elif defined(USE_XPT2046_TOUCH)
        // XPT2046 Resistive Touch (CYD - Cheap Yellow Display)
        if (touchXPT2046.begin()) {
            Serial.println("[Display] Initialized with XPT2046 Touch");
        } else {
            Serial.println("[Display] XPT2046 Touch init failed!");
        }
    #else
        // Fallback: TFT_eSPI built-in touch
        #ifdef TOUCH_CALIBRATION_MODE
            runTouchCalibration();
        #else
            uint16_t calData[5] = { 275, 3620, 230, 3580, 7 };
            tft.setTouch(calData);
        #endif
        Serial.println("[Display] Initialized with TFT_eSPI Touch");
    #endif
}

void initBLE() {
    Serial.println("[BLE] Initializing...");
    
    if (!bleScanner.begin()) {
        Serial.println("[BLE] ERROR: Failed to initialize");
        tft.setTextColor(TFT_RED);
        tft.println("BLE ERROR!");
        while(1) delay(1000);
    }
    
    bleScanner.setRSSIThreshold(BLE_RSSI_THRESHOLD);
    bleScanner.onBeaconDetected(onBeaconDetected);
    
    Serial.println("[BLE] Initialized");
}

void initSD() {
    Serial.println("[SD] Initializing...");
    
    if (!dataLogger.begin(SD_CS_PIN)) {
        Serial.println("[SD] WARNING: SD card not available");
        tft.setTextColor(TFT_YELLOW);
        tft.println("SD: No Card");
        // Continue without SD
    } else {
        Serial.println("[SD] Initialized");
    }
}

void initPersistence() {
    Serial.println("[Persistence] Initializing...");
    
    if (!persistence.begin()) {
        Serial.println("[Persistence] ERROR: Failed to initialize");
    }
    
    // Load teams from NVS
    persistence.loadTeams(lapCounter);
    
    // Load race config
    persistence.loadConfig(currentRaceName, raceDuration);
    raceDuration *= 60 * 1000;  // Convert minutes to milliseconds
    
    Serial.printf("[Persistence] Loaded %u teams\n", lapCounter.getTeamCount());
}

// ============================================================
// Touch Handler
// ============================================================

// Global touch wrapper implementation
bool getTouchCoordinates(uint16_t* x, uint16_t* y) {
    #ifdef USE_GT911_TOUCH
        return touchGT911.getTouch(x, y);
    #elif defined(USE_XPT2046_TOUCH)
        return touchXPT2046.getTouch(x, y);
    #else
        return tft.getTouch(x, y);
    #endif
}

void processTouch() {
    uint16_t x, y;
    
    if (getTouchCoordinates(&x, &y)) {
        // Debounce
        if (millis() - uiState.lastTouchTime < TOUCH_DEBOUNCE) {
            return;
        }
        uiState.lastTouchTime = millis();
        
        Serial.printf("[Touch] X=%u, Y=%u, Screen=%d\n", x, y, uiState.currentScreen);
        
        // Call screen-specific handler
        handleTouch(x, y);
    }
}

// ============================================================
// Screen Dispatcher
// ============================================================

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

// ============================================================
// BLE Callback
// ============================================================

void onBeaconDetected(const BeaconData& beacon) {
    // Check if beacon belongs to a team
    TeamData* team = lapCounter.getTeamByBeacon(beacon.uuid);
    
    if (!team) {
        // Unknown beacon
        Serial.printf("[Beacon] Unknown: %s (RSSI: %d, Dist: %.2fm)\n",
                     beacon.uuid.c_str(), beacon.rssi,
                     BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower));
        return;
    }
    
    // Check proximity
    float distance = BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower);
    Serial.printf("[Beacon] Team %u (%s): RSSI=%d, Dist=%.2fm, Threshold=%.1fm, Race=%s\n",
                 team->teamId, team->teamName.c_str(), beacon.rssi, distance, 
                 BLE_PROXIMITY_THRESHOLD, raceRunning ? "RUNNING" : "STOPPED");
    
    if (distance > BLE_PROXIMITY_THRESHOLD) {
        // Too far
        Serial.printf("[Beacon] Team %u: TOO FAR (%.2fm > %.1fm)\n", 
                     team->teamId, distance, BLE_PROXIMITY_THRESHOLD);
        return;
    }
    
    // Record lap if race is running
    if (raceRunning) {
        Serial.printf("[Beacon] Team %u: Attempting lap record...\n", team->teamId);
        if (lapCounter.recordLap(team->teamId, beacon.lastSeen)) {
            Serial.printf("[Lap] ✅ Team %u (%s): Runde %u erfolgreich gezahlt!\n",
                         team->teamId, team->teamName.c_str(), team->lapCount);
            
            // Log to SD
            if (dataLogger.isReady() && team->laps.size() > 0) {
                LapTime& lap = team->laps.back();
                dataLogger.logLap(team->teamId, team->teamName, 
                                 lap.lapNumber, lap.timestamp, lap.duration);
            }
            
            // Update screen
            uiState.needsRedraw = true;
        } else {
            Serial.printf("[Lap] ❌ Team %u: Runde NICHT gezahlt (zu schnell oder Fehler)\n", team->teamId);
        }
    } else {
        Serial.printf("[Beacon] Team %u: Rennen nicht gestartet, ignoriere Beacon\n", team->teamId);
    }
}

