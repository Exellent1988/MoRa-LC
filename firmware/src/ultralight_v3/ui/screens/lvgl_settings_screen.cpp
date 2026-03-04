#include <Arduino.h>
#include "../../core/config.h"
#include "lvgl_settings_screen.h"
#include "../navigation_lvgl.h"
#include "../widgets/dialog_manager.h"
#include "../../services/persistence_service.h"
#include "../../services/data_logger_service.h"
#include "../../services/lap_counter_service.h"

LVGLSettingsScreen::LVGLSettingsScreen(LVGLDisplay* lvglDisplay, PersistenceService* persistence, DataLoggerService* dataLogger, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _persistence(persistence)
    , _dataLogger(dataLogger)
    , _lapCounter(lapCounter)
    , _debugScreen(nullptr)
    , _list(nullptr)
    , _editModal(nullptr)
    , _editValueLabel(nullptr)
    , _editValue(0)
    , _editMin(0)
    , _editMax(0)
    , _editStep(1)
    , _editKey(0) {
}

LVGLSettingsScreen::~LVGLSettingsScreen() {
    // No cleanup needed - DialogManager is a singleton
}

void LVGLSettingsScreen::setDebugScreen(LVGLDebugScreen* screen) {
    _debugScreen = screen;
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
    
    // Min Lap Time (tap to cycle)
    _minLapTimeItem = lv_list_add_btn(_list, LV_SYMBOL_LOOP, "Min Lap: --s");
    lv_obj_add_event_cb(_minLapTimeItem, minLapTimeEventHandler, LV_EVENT_CLICKED, this);
    updateMinLapTimeLabel();
    
    // RSSI Near threshold (tap to cycle: -50 to -80 in steps of 5)
    _rssiNearItem = lv_list_add_btn(_list, LV_SYMBOL_UPLOAD, "RSSI Near: --");
    lv_obj_add_event_cb(_rssiNearItem, rssiNearEventHandler, LV_EVENT_CLICKED, this);
    updateRssiNearLabel();
    
    // RSSI Far threshold (tap to cycle: -60 to -100 in steps of 5)
    _rssiFarItem = lv_list_add_btn(_list, LV_SYMBOL_DOWNLOAD, "RSSI Far: --");
    lv_obj_add_event_cb(_rssiFarItem, rssiFarEventHandler, LV_EVENT_CLICKED, this);
    updateRssiFarLabel();
    
    lv_obj_t* item1 = lv_list_add_btn(_list, LV_SYMBOL_SETTINGS, "BLE Info");
    lv_obj_set_user_data(item1, (void*)((uintptr_t)1));
    lv_obj_add_event_cb(item1, settingsItemEventHandler, LV_EVENT_CLICKED, this);
    
    lv_obj_t* item2 = lv_list_add_btn(_list, LV_SYMBOL_SAVE, "Save Data");
    lv_obj_set_user_data(item2, (void*)((uintptr_t)2));
    lv_obj_add_event_cb(item2, settingsItemEventHandler, LV_EVENT_CLICKED, this);
    
    lv_obj_t* item4 = lv_list_add_btn(_list, LV_SYMBOL_EYE_OPEN, "Debug");
    lv_obj_set_user_data(item4, (void*)((uintptr_t)4));
    lv_obj_add_event_cb(item4, settingsItemEventHandler, LV_EVENT_CLICKED, this);
    
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
        case 4: // Debug
            if (screen->_navigation && screen->_debugScreen) {
                screen->_navigation->setScreen((LVGLBaseScreen*)screen->_debugScreen);
            }
            break;
    }
}

void LVGLSettingsScreen::handleBLESettings() {
    Serial.println("[LVGLSettings] BLE Settings clicked");
    
    char message[256];
    snprintf(message, sizeof(message), 
             "Interval: %dms / Window: %dms\n"
             "RSSI: %d dBm\n"
             "Filter: %s\n"
             "Timeout: %ds\n\n"
             "Compile-time config.",
             BLE_SCAN_INTERVAL, BLE_SCAN_WINDOW, BLE_RSSI_THRESHOLD,
             BLE_UUID_PREFIX, BEACON_TIMEOUT / 1000);
    
    DialogManager::getInstance()->show("BLE Settings", message, "OK", nullptr, nullptr);
}

void LVGLSettingsScreen::handleSaveData() {
    Serial.println("[LVGLSettings] Save Data clicked");
    
    if (!_persistence) {
        Serial.println("[LVGLSettings] ERROR: PersistenceService not available");
        DialogManager::getInstance()->show("Error", "PersistenceService not available", "OK", nullptr, nullptr);
        return;
    }
    
    if (!_lapCounter) {
        Serial.println("[LVGLSettings] ERROR: LapCounterService not available");
        DialogManager::getInstance()->show("Error", "LapCounterService not available", "OK", nullptr, nullptr);
        return;
    }
    
    // Save teams to persistent storage
    if (_lapCounter->saveTeams(_persistence)) {
        Serial.println("[LVGLSettings] Teams saved successfully");
        DialogManager::getInstance()->show("Success", "Teams saved successfully", "OK", nullptr, nullptr);
    } else {
        Serial.println("[LVGLSettings] ERROR: Failed to save teams");
        DialogManager::getInstance()->show("Error", "Failed to save teams", "OK", nullptr, nullptr);
    }
}

void LVGLSettingsScreen::handleReset() {
    Serial.println("[LVGLSettings] Reset clicked");
    
    if (!_persistence) {
        Serial.println("[LVGLSettings] ERROR: PersistenceService not available");
        DialogManager::getInstance()->show("Error", "PersistenceService not available", "OK", nullptr, nullptr);
        return;
    }
    
    // Show confirmation dialog
    DialogManager::getInstance()->showConfirm("Reset", 
                         "Are you sure you want to delete all data? This cannot be undone.",
                         "Yes", "Cancel",
                         resetConfirmHandler, nullptr, this);
}

void LVGLSettingsScreen::resetConfirmHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (screen && screen->_persistence) {
        screen->_persistence->clearAll();
        if (screen->_lapCounter) {
            screen->_lapCounter->resetRace();
        }
        Serial.println("[LVGLSettings] All data cleared (NVS + in-memory)");
        DialogManager::getInstance()->show("Success", "All data cleared", "OK", nullptr, nullptr);
    }
}

void LVGLSettingsScreen::minLapTimeEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_lapCounter) return;
    int32_t current = (int32_t)(screen->_lapCounter->getMinLapTime() / 1000);
    screen->showValueEditor("Min Lap Time (s)", current, 5, 120, 1, 0);
}

void LVGLSettingsScreen::updateMinLapTimeLabel() {
    if (!_minLapTimeItem || !_lapCounter) return;
    
    uint32_t ms = _lapCounter->getMinLapTime();
    char label[32];
    snprintf(label, sizeof(label), "Min Lap: %lus", ms / 1000);
    
    lv_obj_t* labelObj = lv_obj_get_child(_minLapTimeItem, 1);
    if (labelObj) {
        lv_label_set_text(labelObj, label);
    }
}

void LVGLSettingsScreen::rssiNearEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_lapCounter) return;
    screen->showValueEditor("RSSI Near (dBm)", screen->_lapCounter->getRssiNear(), -90, -30, 1, 1);
}

void LVGLSettingsScreen::rssiFarEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_lapCounter) return;
    screen->showValueEditor("RSSI Far (dBm)", screen->_lapCounter->getRssiFar(), -100, -40, 1, 2);
}

void LVGLSettingsScreen::showValueEditor(const char* title, int32_t current, int32_t min, int32_t max, int32_t step, uint8_t key) {
    _editValue = current;
    _editMin = min;
    _editMax = max;
    _editStep = step;
    _editKey = key;
    
    // Modal background on layer_top
    _editModal = lv_obj_create(lv_layer_top());
    lv_obj_set_size(_editModal, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(_editModal, 0, 0);
    lv_obj_set_style_bg_color(_editModal, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(_editModal, LV_OPA_70, 0);
    lv_obj_clear_flag(_editModal, LV_OBJ_FLAG_SCROLLABLE);
    
    // Dialog box
    lv_obj_t* box = lv_obj_create(_editModal);
    lv_obj_set_size(box, 260, 140);
    lv_obj_align(box, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(box, rgb565ToLVGL(Colors::SURFACE), 0);
    lv_obj_set_style_pad_all(box, 8, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    
    // Title
    lv_obj_t* titleLbl = lv_label_create(box);
    lv_label_set_text(titleLbl, title);
    lv_obj_set_style_text_color(titleLbl, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_align(titleLbl, LV_ALIGN_TOP_MID, 0, 0);
    
    // Value display (large)
    _editValueLabel = lv_label_create(box);
    lv_obj_set_style_text_color(_editValueLabel, rgb565ToLVGL(Colors::TEXT), 0);
    extern const lv_font_t lv_font_montserrat_28;
    lv_obj_set_style_text_font(_editValueLabel, &lv_font_montserrat_28, 0);
    lv_obj_align(_editValueLabel, LV_ALIGN_CENTER, 0, -5);
    updateEditDisplay();
    
    // - button
    lv_obj_t* btnMinus = lv_btn_create(box);
    lv_obj_set_size(btnMinus, 60, 40);
    lv_obj_align(btnMinus, LV_ALIGN_CENTER, -80, -5);
    lv_obj_set_style_bg_color(btnMinus, rgb565ToLVGL(Colors::BUTTON), 0);
    lv_obj_add_event_cb(btnMinus, editMinusHandler, LV_EVENT_CLICKED, this);
    lv_obj_t* lm = lv_label_create(btnMinus);
    lv_label_set_text(lm, "-");
    lv_obj_center(lm);
    lv_obj_set_style_text_color(lm, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    lv_obj_set_style_text_font(lm, &lv_font_montserrat_28, 0);
    
    // + button
    lv_obj_t* btnPlus = lv_btn_create(box);
    lv_obj_set_size(btnPlus, 60, 40);
    lv_obj_align(btnPlus, LV_ALIGN_CENTER, 80, -5);
    lv_obj_set_style_bg_color(btnPlus, rgb565ToLVGL(Colors::BUTTON), 0);
    lv_obj_add_event_cb(btnPlus, editPlusHandler, LV_EVENT_CLICKED, this);
    lv_obj_t* lp = lv_label_create(btnPlus);
    lv_label_set_text(lp, "+");
    lv_obj_center(lp);
    lv_obj_set_style_text_color(lp, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    lv_obj_set_style_text_font(lp, &lv_font_montserrat_28, 0);
    
    // OK button
    lv_obj_t* btnOk = lv_btn_create(box);
    lv_obj_set_size(btnOk, 100, 32);
    lv_obj_align(btnOk, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_bg_color(btnOk, rgb565ToLVGL(Colors::SECONDARY), 0);
    lv_obj_add_event_cb(btnOk, editOkHandler, LV_EVENT_CLICKED, this);
    lv_obj_t* lo = lv_label_create(btnOk);
    lv_label_set_text(lo, "OK");
    lv_obj_center(lo);
    lv_obj_set_style_text_color(lo, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
}

void LVGLSettingsScreen::updateEditDisplay() {
    if (!_editValueLabel) return;
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld", _editValue);
    lv_label_set_text(_editValueLabel, buf);
}

void LVGLSettingsScreen::closeEditor() {
    if (_editModal) {
        lv_obj_del(_editModal);
        _editModal = nullptr;
        _editValueLabel = nullptr;
    }
}

void LVGLSettingsScreen::saveEditedValue() {
    if (!_lapCounter) return;
    
    switch (_editKey) {
        case 0: { // minLapTime (stored in seconds, convert to ms)
            uint32_t ms = (uint32_t)_editValue * 1000;
            _lapCounter->setMinLapTime(ms);
            if (_persistence) _persistence->saveInt("minLapTime", (int32_t)ms);
            updateMinLapTimeLabel();
            Serial.printf("[Settings] Min lap: %lds\n", _editValue);
            break;
        }
        case 1: { // rssiNear
            _lapCounter->setRssiNear((int8_t)_editValue);
            if (_persistence) _persistence->saveInt("rssiNear", _editValue);
            updateRssiNearLabel();
            Serial.printf("[Settings] RSSI Near: %ld\n", _editValue);
            break;
        }
        case 2: { // rssiFar
            _lapCounter->setRssiFar((int8_t)_editValue);
            if (_persistence) _persistence->saveInt("rssiFar", _editValue);
            updateRssiFarLabel();
            Serial.printf("[Settings] RSSI Far: %ld\n", _editValue);
            break;
        }
    }
}

void LVGLSettingsScreen::editMinusHandler(lv_event_t* e) {
    LVGLSettingsScreen* s = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (!s) return;
    if (s->_editValue - s->_editStep >= s->_editMin) {
        s->_editValue -= s->_editStep;
        s->updateEditDisplay();
    }
}

void LVGLSettingsScreen::editPlusHandler(lv_event_t* e) {
    LVGLSettingsScreen* s = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (!s) return;
    if (s->_editValue + s->_editStep <= s->_editMax) {
        s->_editValue += s->_editStep;
        s->updateEditDisplay();
    }
}

void LVGLSettingsScreen::editOkHandler(lv_event_t* e) {
    LVGLSettingsScreen* s = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (!s) return;
    s->saveEditedValue();
    s->closeEditor();
}

void LVGLSettingsScreen::updateRssiNearLabel() {
    if (!_rssiNearItem || !_lapCounter) return;
    char label[32];
    snprintf(label, sizeof(label), "RSSI Near: %d dBm", _lapCounter->getRssiNear());
    lv_obj_t* labelObj = lv_obj_get_child(_rssiNearItem, 1);
    if (labelObj) lv_label_set_text(labelObj, label);
}

void LVGLSettingsScreen::updateRssiFarLabel() {
    if (!_rssiFarItem || !_lapCounter) return;
    char label[32];
    snprintf(label, sizeof(label), "RSSI Far: %d dBm", _lapCounter->getRssiFar());
    lv_obj_t* labelObj = lv_obj_get_child(_rssiFarItem, 1);
    if (labelObj) lv_label_set_text(labelObj, label);
}

void LVGLSettingsScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLSettings] Back button clicked");
        screen->_navigation->goBack();
    }
}

