#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include "display_manager.h"
#include "ui_state.h"
#include "ui_screens.h"
#include "../../lib/BLEScanner/BLEScanner.h"
#include "../../lib/LapCounter/LapCounter.h"
#include "../../lib/DataLogger/DataLogger.h"
#include "../../lib/LapDetector/LapDetector.h"
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

// Lap-Detektoren pro Team (Zugriff nur aus loop(), nicht aus dem BLE-Task!)
std::map<uint8_t, LapDetector> lapDetectors;
std::map<uint64_t, uint8_t> macToTeam;   // MAC (48 bit) -> teamId, wird bei Rennstart gebaut

static uint64_t macKey(const uint8_t mac[6]) {
    uint64_t k = 0;
    for (int i = 5; i >= 0; i--) k = (k << 8) | mac[i];
    return k;
}

static bool parseMac(const String& s, uint64_t& key) {
    unsigned b[6];
    if (sscanf(s.c_str(), "%x:%x:%x:%x:%x:%x", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) != 6) return false;
    key = 0;
    for (int i = 0; i < 6; i++) key = (key << 8) | (b[i] & 0xFF);
    return true;
}

static LapDetectorConfig makeDetectorConfig() {
    LapDetectorConfig c;
    c.nearRssi = lapRssiNear;
    c.farRssi = lapRssiFar;
    c.minLapMs = MIN_LAP_TIME;
    c.lostTimeoutMs = LAP_LOST_TIMEOUT_MS;
    c.smoothing = LAP_RSSI_SMOOTHING;
    return c;
}

// Beim (Wieder-)Start: Zuordnung neu bauen, Detektoren zuruecksetzen
static void prepareRaceTracking() {
    macToTeam.clear();
    lapDetectors.clear();
    for (TeamData* t : lapCounter.getAllTeams()) {
        uint64_t key;
        if (parseMac(t->beaconUUID, key)) {
            macToTeam[key] = t->teamId;
            lapDetectors[t->teamId] = LapDetector(makeDetectorConfig());
        }
    }
    Serial.printf("[Race] Tracking %u teams\n", (unsigned)macToTeam.size());
}

static void handleLap(TeamData* team, uint32_t lapTs) {
    if (!lapCounter.recordLap(team->teamId, lapTs)) return;

    Serial.printf("[Lap] Team %u (%s): Lap %u\n",
                  team->teamId, team->teamName.c_str(), team->lapCount);

    if (dataLogger.isReady() && !team->laps.empty()) {
        LapTime& lap = team->laps.back();
        dataLogger.logLap(team->teamId, team->teamName,
                          lap.lapNumber, lap.timestamp, lap.duration);
    }
    uiState.needsRedraw = true;
}

// Alle Pakete aus der BLE-Queue abarbeiten (im Loop, nicht im BLE-Task)
static void processBeaconSamples() {
    static bool wasRunning = false;
    if (raceRunning && !wasRunning) prepareRaceTracking();
    wasRunning = raceRunning;

    BeaconSample sample;
    char macStr[18];
    uint8_t budget = 100;  // pro Loop-Durchlauf begrenzen, damit Touch/UI nicht haengen
    while (budget-- && bleScanner.popSample(sample)) {
        if (!raceRunning) continue;  // ausserhalb des Rennens verwerfen

        BLEScanner::macToString(sample.mac, macStr);
        dataLogger.logRaw(sample.ts, macStr, sample.rssi);

        auto tm = macToTeam.find(macKey(sample.mac));
        if (tm == macToTeam.end()) continue;
        TeamData* team = lapCounter.getTeam(tm->second);
        if (!team) continue;

        uint32_t lapTs;
        if (lapDetectors[team->teamId].onSample(sample.ts, sample.rssi, lapTs)) {
            handleLap(team, lapTs);
        }
    }

    if (raceRunning) {
        // Beacon verschwunden -> Durchfahrt abschliessen
        uint32_t now = millis();
        for (auto& kv : lapDetectors) {
            uint32_t lapTs;
            if (kv.second.tick(now, lapTs)) {
                TeamData* team = lapCounter.getTeam(kv.first);
                if (team) handleLap(team, lapTs);
            }
        }
        dataLogger.serviceRaw();
    }
}

// Alle 5 s eine Diagnosezeile (Pakete, Queue-Verluste, SD-Fehler)
static void printDiagnostics() {
    static uint32_t last = 0;
    static uint32_t lastSamples = 0;
    if (!raceRunning || millis() - last < 5000) return;
    last = millis();
    uint32_t total = 0;
    for (auto& kv : lapDetectors) total += kv.second.sampleCount();
    Serial.printf("[Diag] pkts/5s=%u dropped=%u sdErr=%u laps=%u heap=%u\n",
                  total - lastSamples, bleScanner.droppedSamples(),
                  dataLogger.getWriteErrors(), dataLogger.getLapsLogged(), ESP.getFreeHeap());
    lastSamples = total;
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
    
    // SD nach Display-Init erneut pruefen (gleicher SPI-Host!) und ggf. neu initialisieren
    if (dataLogger.isReady() && !dataLogger.selfTest()) {
        Serial.println("[SD] Self-test failed after display init, re-initializing...");
        dataLogger.begin(SD_CS_PIN);
        if (!dataLogger.selfTest()) Serial.println("[SD] ERROR: SD unusable");
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

// Forward declaration
void drawScreen();

void loop() {
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
    
    // BLE-Pakete auswerten (Lap-Erkennung + Roh-Log); Beacon-Liste nur ausserhalb des Rennens pflegen
    bleScanner.setListEnabled(!raceRunning);
    processBeaconSamples();
    printDiagnostics();
    
    // Screen redraw if needed
    if (uiState.needsRedraw) {
        drawScreen();
        uiState.needsRedraw = false;
    }
    
    // BLE scanning - start continuous scan if not already scanning (only when needed)
    if (!raceRunning && !bleScanner.isScanning()) {
        // Only scan when in beacon assignment or race setup
        if (uiState.currentScreen == SCREEN_TEAM_BEACON_ASSIGN || 
            uiState.currentScreen == SCREEN_BEACON_LIST ||
            uiState.currentScreen == SCREEN_RACE_SETUP) {
            // Re-disable logging before starting scan
            esp_log_level_set("*", ESP_LOG_NONE);
            esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
            bleScanner.startScan(0);  // 0 = continuous
        }
    } else if (raceRunning && !bleScanner.isScanning()) {
        // Re-disable logging before starting scan
        esp_log_level_set("*", ESP_LOG_NONE);
        esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
        bleScanner.startScan(0);
    }
    
    // Periodically re-disable logging (NimBLE might reset it)
    static uint32_t lastLogDisable = 0;
    if (millis() - lastLogDisable > 5000) {  // Every 5 seconds
        esp_log_level_set("*", ESP_LOG_NONE);
        esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
        esp_log_level_set("NimBLE", ESP_LOG_NONE);
        lastLogDisable = millis();
    }
    
    // Clean up old beacons (only every second to reduce overhead)
    static uint32_t lastBeaconCleanup = 0;
    if (millis() - lastBeaconCleanup > 1000) {
        bleScanner.clearOldBeacons(BEACON_TIMEOUT);
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
