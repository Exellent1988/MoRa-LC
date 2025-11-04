#include "lvgl_settings_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLSettingsScreen::LVGLSettingsScreen(LVGLDisplay* lvglDisplay)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
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
    
    // Create header with back button
    createHeader("Einstellungen", true, backBtnEventHandler, this);
    
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
    
    // TODO: Add actual settings items (no umlauts for now - LVGL default font doesn't support them)
    lv_obj_t* item1 = lv_list_add_btn(_list, LV_SYMBOL_SETTINGS, "BLE Einstellungen");
    lv_obj_t* item2 = lv_list_add_btn(_list, LV_SYMBOL_SAVE, "Daten speichern");
    lv_obj_t* item3 = lv_list_add_btn(_list, LV_SYMBOL_REFRESH, "Zuruecksetzen");
}

void LVGLSettingsScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLSettingsScreen* screen = (LVGLSettingsScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLSettings] Back button clicked");
        screen->_navigation->goBack();
    }
}

