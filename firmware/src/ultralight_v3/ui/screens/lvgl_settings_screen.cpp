#include "lvgl_settings_screen.h"
#include "../navigation_lvgl.h"
#include "../widgets/lvgl_dialog.h"
#include "../../services/persistence_service.h"
#include "../../services/data_logger_service.h"
#include "../../services/lap_counter_service.h"
#include "../../core/config.h"
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
    
    if (!_screen) return;
    
    // Show BLE settings info dialog
    LVGLDialog dialog(_screen);
    
    // Get BLE status info (simplified - just show that BLE is available)
    char message[256];
    snprintf(message, sizeof(message), 
             "BLE Scanner:\n"
             "Mode: Active\n"
             "RSSI Threshold: %d dBm\n"
             "Scan Interval: %d ms\n"
             "\n"
             "To configure BLE settings,\n"
             "use config.h and rebuild.",
             BLE_RSSI_THRESHOLD, BLE_SCAN_INTERVAL);
    
    dialog.show("BLE Settings", message, "OK", nullptr, nullptr);
}

void LVGLSettingsScreen::handleSaveData() {
    Serial.println("[LVGLSettings] Save Data clicked");
    
    if (!_screen) return;
    
    LVGLDialog dialog(_screen);
    
    if (_persistence && _lapCounter) {
        if (_lapCounter->saveTeams(_persistence)) {
            Serial.println("[LVGLSettings] Teams saved successfully");
            dialog.show("Save Data", "Teams saved\nsuccessfully!", "OK", nullptr, nullptr);
        } else {
            Serial.println("[LVGLSettings] ERROR: Failed to save teams");
            dialog.show("Save Data", "Failed to save teams.\nPlease try again.", "OK", nullptr, nullptr);
        }
    } else {
        Serial.println("[LVGLSettings] ERROR: PersistenceService or LapCounterService not available");
        dialog.show("Save Data", "Services not available.", "OK", nullptr, nullptr);
    }
}

void LVGLSettingsScreen::handleReset() {
    Serial.println("[LVGLSettings] Reset clicked");
    
    if (!_screen) return;
    
    // Show confirmation dialog
    LVGLDialog dialog(_screen);
    
    struct ResetData {
        LVGLSettingsScreen* screen;
    };
    ResetData* resetData = new ResetData{this};
    
    dialog.showConfirm(
        "Reset All Data",
        "This will delete all teams\nand settings.\n\nContinue?",
        "Reset",
        "Cancel",
        resetConfirmOkCallback,
        resetConfirmCancelCallback,
        resetData
    );
}

void LVGLSettingsScreen::resetConfirmOkCallback(lv_event_t* e) {
    struct ResetData {
        LVGLSettingsScreen* screen;
    };
    ResetData* data = (ResetData*)lv_event_get_user_data(e);
    if (data && data->screen && data->screen->_persistence) {
        data->screen->_persistence->clearAll();
        Serial.println("[LVGLSettings] All data cleared");
    }
    delete data;
}

void LVGLSettingsScreen::resetConfirmCancelCallback(lv_event_t* e) {
    struct ResetData {
        LVGLSettingsScreen* screen;
    };
    ResetData* data = (ResetData*)lv_event_get_user_data(e);
    delete data;
}

void LVGLSettingsScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLSettings] Back button clicked");
        screen->_navigation->goBack();
    }
}

