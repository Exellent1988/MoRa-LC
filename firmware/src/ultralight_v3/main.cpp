#include <Arduino.h>
#include "core/config.h"
#include "core/system.h"

// BLE Library Selection
#ifdef BLE_USE_ESP32
#include "hardware/ble_esp32.h"
#elif defined(BLE_USE_ESP_IDF)
#include "hardware/ble_esp_idf.h"
#else
#include "hardware/ble_nimble.h"
#endif

#include "services/beacon_service.h"
#include "services/lap_counter_service.h"
#include "services/persistence_service.h"
#include "services/data_logger_service.h"

// Only include Display/UI if not in Serial-Only mode
#ifndef SERIAL_ONLY_MODE
#include "hardware/display.h"
#include "hardware/touch.h"
#include "hardware/sd_card.h"
#include "hardware/lvgl_display.h"
#include "ui/navigation_lvgl.h"
#include "ui/screens/lvgl_base_screen.h"
#include "ui/screens/lvgl_home_screen.h"
#include "ui/screens/lvgl_ble_test_screen.h"
#include "ui/screens/lvgl_teams_screen.h"
#include "ui/screens/lvgl_race_setup_screen.h"
#include "ui/screens/lvgl_race_results_screen.h"
#include "ui/screens/lvgl_race_running_screen.h"
#include "ui/screens/lvgl_settings_screen.h"
#include "ui/screens/lvgl_beacon_assign_screen.h"
#include "ui/screens/lvgl_team_edit_screen.h"
#include "ui/screens/lvgl_test_screen.h"
#endif

// Global instances
SystemManager systemManager;

// BLE - Conditional compilation based on config
#ifdef BLE_USE_ESP32
BLEESP32 bleInterface;
#elif defined(BLE_USE_ESP_IDF)
BLEESPIDF bleInterface;
#else
BLENimBLE bleInterface;
#endif
BeaconService beaconService;
LapCounterService lapCounterService;
PersistenceService persistenceService;
DataLoggerService dataLoggerService;

#ifndef SERIAL_ONLY_MODE
Display display;
TouchHandler touchHandler;
SDCard sdCard;
LVGLDisplay lvglDisplay(&display);
LVGLNavigation lvglNavigation;
LVGLHomeScreen lvglHomeScreen(&lvglDisplay);
LVGLBLETestScreen lvglBLETestScreen(&lvglDisplay, &beaconService);
LVGLTeamsScreen lvglTeamsScreen(&lvglDisplay, &lapCounterService);
LVGLRaceSetupScreen lvglRaceSetupScreen(&lvglDisplay);
LVGLRaceRunningScreen lvglRaceRunningScreen(&lvglDisplay, &beaconService, &lapCounterService);
LVGLRaceResultsScreen lvglRaceResultsScreen(&lvglDisplay, &lapCounterService);
LVGLSettingsScreen lvglSettingsScreen(&lvglDisplay, &persistenceService, &dataLoggerService);
LVGLBeaconAssignScreen lvglBeaconAssignScreen(&lvglDisplay, &beaconService, &lapCounterService);
LVGLTeamEditScreen lvglTeamEditScreen(&lvglDisplay, &lapCounterService);
#endif

// ============================================================
// Setup
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=================================");
    Serial.println("MoRa-LC UltraLight v3.0.0");
    #ifdef SERIAL_ONLY_MODE
    Serial.println("SERIAL ONLY MODE - No Display");
    #endif
    Serial.println("=================================\n");
    
    // Initialize system
    if (!systemManager.initialize()) {
        Serial.println("[Setup] ERROR: System initialization failed!");
        while(1) delay(1000);
    }
    
    #ifdef SERIAL_ONLY_MODE
    // SERIAL ONLY MODE: Skip Display/UI, test BLE directly
    Serial.println("[Main] Serial-Only Mode: Skipping Display/UI");
    
    // Initialize BLE
    Serial.println("[Main] Initializing BLE...");
    #ifdef BLE_USE_ESP32
    Serial.println("[Main] Using ESP32 BLE Arduino Library");
    #elif defined(BLE_USE_ESP_IDF)
    Serial.println("[Main] Using ESP-IDF Native BLE API");
    #else
    Serial.println("[Main] Using NimBLE Library");
    #endif
    
    if (bleInterface.begin()) {
        bleInterface.setMACFilter(BLE_UUID_PREFIX);
        bleInterface.setRSSIThreshold(BLE_RSSI_THRESHOLD);
        Serial.println("[Main] BLE initialized");
        
        if (beaconService.begin(&bleInterface)) {
            Serial.println("[Main] BeaconService initialized");
            
            // Enable race mode for continuous updates
            beaconService.setRaceMode(true);
            
            // Set beacon callback for Serial output
            beaconService.setBeaconCallback([](const BeaconInfo& beacon) {
                Serial.printf("[Beacon] MAC=%s, RSSI=%d dBm (avg=%d), UUID=%s, Major=%u, Minor=%u\n",
                             beacon.macAddress.c_str(), beacon.rssi, beacon.avgRssi,
                             beacon.uuid.c_str(), beacon.major, beacon.minor);
            });
            
            // Start scanning immediately
            Serial.println("[Main] Starting BLE scan...");
            beaconService.startScanning(0);
            Serial.println("[Main] BLE scanning started - waiting for beacons...\n");
        } else {
            Serial.println("[Main] ERROR: BeaconService failed");
        }
    } else {
        Serial.println("[Main] ERROR: BLE initialization failed");
    }
    
    #else
    // NORMAL MODE: With Display/UI
    // Initialize hardware in order
    // 1. SD Card (must be first - uses VSPI)
    sdCard.begin(SD_CS_PIN);
    
    // 2. Display
    if (!display.begin()) {
        Serial.println("[Setup] ERROR: Display initialization failed!");
        while(1) delay(1000);
    }
    
    // 3. LVGL Display (LVGL wrapper around Display)
    Serial.println("[Main] Initializing LVGL...");
    if (!lvglDisplay.begin()) {
        Serial.println("[Setup] ERROR: LVGL initialization failed!");
        while(1) delay(1000);
    }
    
    // 4. Initialize LVGL Navigation
    lvglNavigation.begin(&lvglDisplay);
    Serial.println("[Main] LVGL Navigation initialized");
    
    // 5. Set up screen navigation
    lvglHomeScreen.setNavigation(&lvglNavigation);
    lvglHomeScreen.setTeamsScreen(&lvglTeamsScreen);
    lvglHomeScreen.setRaceSetupScreen(&lvglRaceSetupScreen);
    lvglHomeScreen.setRaceResultsScreen(&lvglRaceResultsScreen);
    lvglHomeScreen.setSettingsScreen(&lvglSettingsScreen);
    
    lvglBLETestScreen.setNavigation(&lvglNavigation);
    lvglTeamsScreen.setNavigation(&lvglNavigation);
    lvglTeamsScreen.setBeaconAssignScreen(&lvglBeaconAssignScreen);
    lvglTeamsScreen.setTeamEditScreen(&lvglTeamEditScreen);
    lvglBeaconAssignScreen.setNavigation(&lvglNavigation);
    lvglTeamEditScreen.setNavigation(&lvglNavigation);
    lvglRaceSetupScreen.setNavigation(&lvglNavigation);
    lvglRaceSetupScreen.setRaceRunningScreen(&lvglRaceRunningScreen);
    lvglRaceRunningScreen.setNavigation(&lvglNavigation);
    lvglRaceRunningScreen.setRaceResultsScreen(&lvglRaceResultsScreen);
    lvglRaceResultsScreen.setNavigation(&lvglNavigation);
    lvglSettingsScreen.setNavigation(&lvglNavigation);
    
    // 6. Show initial screen
    #ifdef BLE_TEST_MODE
    Serial.println("[Main] Starting with BLE Test Screen");
    lvglNavigation.setScreen(&lvglBLETestScreen);
    #else
    Serial.println("[Main] Starting with Home Screen");
    lvglNavigation.setScreen(&lvglHomeScreen);
    #endif
    
    // 7. Initialize BLE
    Serial.println("[Main] Initializing BLE...");
    #ifdef BLE_USE_ESP32
    Serial.println("[Main] Using ESP32 BLE Arduino Library");
    #else
    Serial.println("[Main] Using NimBLE Library");
    #endif
    
    if (bleInterface.begin()) {
        bleInterface.setMACFilter(BLE_UUID_PREFIX);
        bleInterface.setRSSIThreshold(BLE_RSSI_THRESHOLD);
        Serial.println("[Main] BLE initialized");
        
        if (beaconService.begin(&bleInterface)) {
            Serial.println("[Main] BeaconService initialized");
            
            // Initialize services
            if (lapCounterService.begin(&beaconService)) {
                Serial.println("[Main] LapCounterService initialized");
                
                // Load teams from persistence
                if (persistenceService.begin()) {
                    Serial.println("[Main] PersistenceService initialized");
                    lapCounterService.loadTeams(&persistenceService);
                }
            }
            
            if (dataLoggerService.begin(&sdCard)) {
                Serial.println("[Main] DataLoggerService initialized");
            }
            
            // Set beacon callback for testing (only Serial, no Display!)
            beaconService.setBeaconCallback([](const BeaconInfo& beacon) {
                Serial.printf("[Beacon] MAC=%s, RSSI=%d dBm (avg=%d), UUID=%s, Major=%u, Minor=%u\n",
                             beacon.macAddress.c_str(), beacon.rssi, beacon.avgRssi,
                             beacon.uuid.c_str(), beacon.major, beacon.minor);
            });
        } else {
            Serial.println("[Main] ERROR: BeaconService failed");
        }
    } else {
        Serial.println("[Main] ERROR: BLE initialization failed");
    }
    
    // 6. LVGL Test Screen is already created above
    // No need for old Navigation/Screen system - we're using LVGL now
    #endif
    
    Serial.println("\nSetup complete!");
}

// ============================================================
// Loop
// ============================================================

void loop() {
    #ifdef SERIAL_ONLY_MODE
    // Serial-Only Mode: Just update beacon service
    // (BeaconService handles BLE callbacks automatically)
    
    // Feed watchdog frequently (every 100ms)
    systemManager.feedWatchdog();
    
    // NimBLE with setDuplicateFilter(false) provides continuous updates without restart
    // No need to restart scan frequently - NimBLE handles duplicates correctly
    
    delay(100);
    
    // Feed watchdog again after delay
    systemManager.feedWatchdog();
    
    // Print beacon count every 5 seconds
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 5000) {
        auto beacons = beaconService.getBeacons();
        Serial.printf("[Status] Active beacons: %zu, Scanning: %s\n", 
                     beacons.size(), bleInterface.isScanning() ? "YES" : "NO");
        lastPrint = millis();
    }
    #else
    // Normal Mode: Update LVGL (handles screen updates and touch)
    // Feed watchdog
    systemManager.feedWatchdog();
    
    // Update LVGL (this handles all rendering and touch events)
    lvglDisplay.update();
    
    // Update current screen (for dynamic updates)
    LVGLBaseScreen* currentScreen = lvglNavigation.getCurrentScreen();
    if (currentScreen) {
        currentScreen->update();
    }
    
    // Update services
    lapCounterService.update();
    dataLoggerService.update();
    
    // Small delay to prevent 100% CPU usage
    delay(5);  // 5ms for smoother LVGL updates
    #endif
}

