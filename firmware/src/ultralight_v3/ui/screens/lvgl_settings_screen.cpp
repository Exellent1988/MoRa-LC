#include "lvgl_settings_screen.h"
#include "../navigation_lvgl.h"
#include "../../services/persistence_service.h"
#include "../../services/data_logger_service.h"
#include "../../services/lap_counter_service.h"
#include "../../services/beacon_service.h"
#include <Arduino.h>

LVGLSettingsScreen::LVGLSettingsScreen(LVGLDisplay* lvglDisplay,
                                       PersistenceService* persistence,
                                       DataLoggerService* dataLogger,
                                       LapCounterService* lapCounter,
                                       BeaconService* beaconService)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _persistence(persistence)
    , _dataLogger(dataLogger)
    , _lapCounter(lapCounter)
    , _beaconService(beaconService)
    , _list(nullptr)
    , _dialog(nullptr)
    , _bleDialog(nullptr)
    , _bleSlider(nullptr)
    , _bleValueLabel(nullptr)
    , _bleScanSwitch(nullptr) {
}

LVGLSettingsScreen::~LVGLSettingsScreen() {
    closeBLEDialog();
    if (_dialog) {
        delete _dialog;
        _dialog = nullptr;
    }
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
    
    closeBLEDialog();
    if (_dialog) {
        delete _dialog;
        _dialog = nullptr;
    }
    _dialog = new LVGLDialog(_screen);

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
    closeBLEDialog();
    if (_dialog) {
        delete _dialog;
        _dialog = nullptr;
    }
}

void LVGLSettingsScreen::createSettingsList() {
    if (!_list) return;
    
    // Add settings items (ASCII only - no umlauts)
    // Store screen pointer in user data for event handler
    lv_obj_t* item1 = lv_list_add_btn(_list, LV_SYMBOL_SETTINGS, "BLE Settings");
    styleListItem(item1, Fonts::Size::Body);
    lv_obj_set_user_data(item1, (void*)((uintptr_t)1));
    lv_obj_add_event_cb(item1, settingsItemEventHandler, LV_EVENT_CLICKED, this);
    
    lv_obj_t* item2 = lv_list_add_btn(_list, LV_SYMBOL_SAVE, "Save Teams");
    styleListItem(item2, Fonts::Size::Body);
    lv_obj_set_user_data(item2, (void*)((uintptr_t)2));
    lv_obj_add_event_cb(item2, settingsItemEventHandler, LV_EVENT_CLICKED, this);
    
    lv_obj_t* item3 = lv_list_add_btn(_list, LV_SYMBOL_REFRESH, "Reset Storage");
    styleListItem(item3, Fonts::Size::Body);
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
    closeBLEDialog();

    _bleDialog = lv_obj_create(_screen);
    lv_obj_set_size(_bleDialog, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(_bleDialog, 0, 0);
    lv_obj_set_style_bg_color(_bleDialog, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(_bleDialog, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_border_width(_bleDialog, 0, LV_PART_MAIN);

    lv_obj_t* panel = lv_obj_create(_bleDialog);
    lv_obj_set_size(panel, SCREEN_WIDTH - 60, SCREEN_HEIGHT - 80);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(panel, rgb565ToLVGL(Colors::SURFACE), LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, rgb565ToLVGL(Colors::BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, Spacing::SM, LV_PART_MAIN);
    lv_obj_set_style_pad_all(panel, Spacing::MD, LV_PART_MAIN);

    // Title
    lv_obj_t* title = lv_label_create(panel);
    lv_label_set_text(title, "BLE Settings");
    lv_obj_set_style_text_color(title, rgb565ToLVGL(Colors::TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, Fonts::get(Fonts::Size::Title), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    // Close button
    lv_obj_t* closeBtn = lv_btn_create(panel);
    lv_obj_set_size(closeBtn, 40, 40);
    lv_obj_align(closeBtn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(closeBtn, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(closeBtn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(closeBtn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(closeBtn, bleDialogCloseHandler, LV_EVENT_CLICKED, this);

    lv_obj_t* closeLabel = lv_label_create(closeBtn);
    lv_label_set_text(closeLabel, "X");
    lv_obj_set_style_text_color(closeLabel, rgb565ToLVGL(Colors::TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(closeLabel, Fonts::get(Fonts::Size::Subtitle), LV_PART_MAIN);
    lv_obj_center(closeLabel);

    int y = Spacing::LG;

    // RSSI Threshold label
    _bleValueLabel = lv_label_create(panel);
    lv_obj_set_style_text_color(_bleValueLabel, rgb565ToLVGL(Colors::TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(_bleValueLabel, Fonts::get(Fonts::Size::Body), LV_PART_MAIN);
    lv_obj_align(_bleValueLabel, LV_ALIGN_TOP_LEFT, 0, y);

    y += Spacing::LG;

    // Slider
    _bleSlider = lv_slider_create(panel);
    lv_obj_set_size(_bleSlider, lv_obj_get_width(panel) - Spacing::SM * 2, 20);
    lv_obj_align(_bleSlider, LV_ALIGN_TOP_LEFT, 0, y);
    lv_slider_set_range(_bleSlider, -100, -40);
    int16_t currentThreshold = BLE_RSSI_THRESHOLD;
    if (_beaconService) {
        currentThreshold = _beaconService->getRSSIThreshold();
    }
    lv_slider_set_value(_bleSlider, currentThreshold, LV_ANIM_OFF);
    lv_obj_add_event_cb(_bleSlider, bleThresholdChangedHandler, LV_EVENT_VALUE_CHANGED, this);

    y += Spacing::LG * 2;

    // Scanning switch
    lv_obj_t* scanLabel = lv_label_create(panel);
    lv_label_set_text(scanLabel, "Continuous Scanning");
    lv_obj_set_style_text_color(scanLabel, rgb565ToLVGL(Colors::TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(scanLabel, Fonts::get(Fonts::Size::Body), LV_PART_MAIN);
    lv_obj_align(scanLabel, LV_ALIGN_TOP_LEFT, 0, y);
    
    _bleScanSwitch = lv_switch_create(panel);
    lv_obj_align(_bleScanSwitch, LV_ALIGN_TOP_RIGHT, 0, y - Spacing::XS);
    if (_beaconService && _beaconService->isScanning()) {
        lv_obj_add_state(_bleScanSwitch, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(_bleScanSwitch, bleScanToggleHandler, LV_EVENT_VALUE_CHANGED, this);

    // Info text
    lv_obj_t* info = lv_label_create(panel);
    lv_label_set_text(info, "Adjust RSSI threshold for lap detection. Lower values = more sensitive.");
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(info, lv_obj_get_width(panel) - Spacing::SM * 2);
    lv_obj_set_style_text_color(info, rgb565ToLVGL(Colors::TEXT_SECONDARY), LV_PART_MAIN);
    lv_obj_set_style_text_font(info, Fonts::get(Fonts::Size::Caption), LV_PART_MAIN);
    lv_obj_align(info, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    updateBLEValueLabel(currentThreshold);
}

void LVGLSettingsScreen::handleSaveData() {
    Serial.println("[LVGLSettings] Save Data clicked");
    // Note: Teams need to be saved via LapCounterService
    // This is just a placeholder - actual save would need LapCounterService reference
    if (!_dialog) {
        _dialog = new LVGLDialog(_screen);
    }

    if (_lapCounter && _persistence) {
        bool result = _lapCounter->saveTeams(_persistence);
        if (result) {
            Serial.println("[LVGLSettings] Teams saved");
            _dialog->show("Saved", "Teams saved to persistence.");
        } else {
            Serial.println("[LVGLSettings] ERROR: Saving teams failed");
            _dialog->show("Error", "Could not save teams.");
        }
    } else {
        Serial.println("[LVGLSettings] ERROR: PersistenceService or LapCounter not available");
        _dialog->show("Error", "Persistence or LapCounter missing.");
    }
}

void LVGLSettingsScreen::handleReset() {
    Serial.println("[LVGLSettings] Reset clicked");
    if (!_dialog) {
        _dialog = new LVGLDialog(_screen);
    }

    if (!_persistence) {
        Serial.println("[LVGLSettings] ERROR: PersistenceService not available");
        _dialog->show("Error", "Persistence not available.");
        return;
    }

    _dialog->showConfirm("Reset Storage",
                         "Clear all saved teams and settings?",
                         "Clear", "Cancel",
                         [](lv_event_t* e) {
                             LVGLSettingsScreen* screen = static_cast<LVGLSettingsScreen*>(lv_event_get_user_data(e));
                             if (!screen || !screen->_persistence) return;
                             screen->_persistence->clearAll();
                             if (screen->_lapCounter) {
                                 screen->_lapCounter->resetRace();
                             }
                             Serial.println("[LVGLSettings] All data cleared");
                             if (screen->_dialog) {
                                 screen->_dialog->show("Reset Complete", "Persistent data cleared.");
                             }
                         },
                         nullptr,
                         this);
}

void LVGLSettingsScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLSettings] Back button clicked");
        screen->_navigation->goBack();
    }
}

void LVGLSettingsScreen::bleDialogCloseHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = static_cast<LVGLSettingsScreen*>(lv_event_get_user_data(e));
    if (!screen) return;
    screen->closeBLEDialog();
}

void LVGLSettingsScreen::bleThresholdChangedHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = static_cast<LVGLSettingsScreen*>(lv_event_get_user_data(e));
    if (!screen) return;
    lv_obj_t* slider = lv_event_get_target(e);
    if (!slider) return;
    int16_t threshold = (int16_t)lv_slider_get_value(slider);
    screen->updateBLEValueLabel(threshold);
    if (screen->_beaconService) {
        screen->_beaconService->setRSSIThreshold((int8_t)threshold);
    }
}

void LVGLSettingsScreen::bleScanToggleHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = static_cast<LVGLSettingsScreen*>(lv_event_get_user_data(e));
    if (!screen || !screen->_beaconService) return;
    lv_obj_t* sw = lv_event_get_target(e);
    if (!sw) return;
    bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
    if (enabled) {
        screen->_beaconService->startScanning(0);
    } else {
        screen->_beaconService->stopScanning();
    }
}

void LVGLSettingsScreen::closeBLEDialog() {
    if (_bleDialog) {
        lv_obj_del(_bleDialog);
        _bleDialog = nullptr;
    }
    _bleSlider = nullptr;
    _bleValueLabel = nullptr;
    _bleScanSwitch = nullptr;
}

void LVGLSettingsScreen::updateBLEValueLabel(int16_t threshold) {
    if (!_bleValueLabel) return;
    char buf[48];
    snprintf(buf, sizeof(buf), "RSSI Threshold: %d dBm", threshold);
    lv_label_set_text(_bleValueLabel, buf);
}

