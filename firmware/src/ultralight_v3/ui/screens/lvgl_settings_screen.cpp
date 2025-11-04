#include "lvgl_settings_screen.h"
#include "../navigation_lvgl.h"
#include "../../services/persistence_service.h"
#include "../../services/data_logger_service.h"
#include "../../services/lap_counter_service.h"
#include <Arduino.h>

LVGLSettingsScreen::LVGLSettingsScreen(LVGLDisplay* lvglDisplay, PersistenceService* persistence, DataLoggerService* dataLogger, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _persistence(persistence)
    , _dataLogger(dataLogger)
    , _lapCounter(lapCounter)
    , _list(nullptr) {
}

LVGLSettingsScreen::~LVGLSettingsScreen() {
}

void LVGLSettingsScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLSettings] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLSettings] Creating settings screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header with back button (ASCII only - no umlauts)
    createHeader("Settings", true, backBtnEventHandler, this);
    
    // Create list for settings
    int listY = HEADER_HEIGHT + Spacing::SM;
    int listH = SCREEN_HEIGHT - listY - Spacing::MD;
    _list = createList(Spacing::MD, listY, SCREEN_WIDTH - 2 * Spacing::MD, listH);
    
    // Create settings list
    createSettingsList();
    
    Serial.println("[LVGLSettings] Settings screen created");
}

void LVGLSettingsScreen::onExit() {
    // Cleanup if needed
}

void LVGLSettingsScreen::createSettingsList() {
    if (!_list) return;
    
    // Add settings items (ASCII only - no umlauts)
    // Store screen pointer in user data for event handler
    lv_obj_t* item1 = lv_list_add_btn(_list, LV_SYMBOL_SETTINGS, "BLE Settings");
    lv_obj_set_user_data(item1, (void*)((uintptr_t)1));
    lv_obj_add_event_cb(item1, settingsItemEventHandler, LV_EVENT_CLICKED, this);
    
    lv_obj_t* item2 = lv_list_add_btn(_list, LV_SYMBOL_SAVE, "Save Data");
    lv_obj_set_user_data(item2, (void*)((uintptr_t)2));
    lv_obj_add_event_cb(item2, settingsItemEventHandler, LV_EVENT_CLICKED, this);
    
    lv_obj_t* item3 = lv_list_add_btn(_list, LV_SYMBOL_REFRESH, "Reset");
    lv_obj_set_user_data(item3, (void*)((uintptr_t)3));
    lv_obj_add_event_cb(item3, settingsItemEventHandler, LV_EVENT_CLICKED, this);
}

void LVGLSettingsScreen::settingsItemEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (!screen) return;
    
    lv_obj_t* obj = lv_event_get_target(e);
    uintptr_t action = (uintptr_t)lv_obj_get_user_data(obj);
    
    switch(action) {
        case 1: // BLE Settings
            screen->handleBLESettings();
            break;
        case 2: // Save Data
            screen->handleSaveData();
            break;
        case 3: // Reset
            screen->handleReset();
            break;
    }
}

void LVGLSettingsScreen::handleBLESettings() {
    Serial.println("[LVGLSettings] BLE Settings clicked");
    
    // Show BLE settings info (read-only for now)
    // In a full implementation, this would open a dialog with adjustable settings
    
    Serial.println("[LVGLSettings] BLE Configuration:");
    Serial.printf("  - Scan Interval: %d ms\n", BLE_SCAN_INTERVAL);
    Serial.printf("  - Scan Window: %d ms\n", BLE_SCAN_WINDOW);
    Serial.printf("  - RSSI Threshold: %d dBm\n", BLE_RSSI_THRESHOLD);
    Serial.printf("  - UUID Filter: %s\n", BLE_UUID_PREFIX);
    Serial.printf("  - Beacon Timeout: %d ms\n", BEACON_TIMEOUT);
    
    // TODO: Implement adjustable BLE settings with LVGL dialog
    // For now, settings are configured via config.h at compile time
}

void LVGLSettingsScreen::handleSaveData() {
    Serial.println("[LVGLSettings] Save Data clicked");
    
    if (!_persistence) {
        Serial.println("[LVGLSettings] ERROR: PersistenceService not available");
        return;
    }
    
    if (!_lapCounter) {
        Serial.println("[LVGLSettings] ERROR: LapCounterService not available");
        return;
    }
    
    // Save teams to persistent storage
    if (_lapCounter->saveTeams(_persistence)) {
        Serial.println("[LVGLSettings] Teams saved successfully");
    } else {
        Serial.println("[LVGLSettings] ERROR: Failed to save teams");
    }
}

void LVGLSettingsScreen::handleReset() {
    Serial.println("[LVGLSettings] Reset clicked");
    // TODO: Show confirmation dialog
    if (_persistence) {
        _persistence->clearAll();
        Serial.println("[LVGLSettings] All data cleared");
    } else {
        Serial.println("[LVGLSettings] ERROR: PersistenceService not available");
    }
}

void LVGLSettingsScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLSettings] Back button clicked");
        screen->_navigation->goBack();
    }
}

