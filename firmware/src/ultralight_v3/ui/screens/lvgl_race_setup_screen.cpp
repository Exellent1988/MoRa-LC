#include "lvgl_race_setup_screen.h"
#include "lvgl_race_running_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLRaceSetupScreen::LVGLRaceSetupScreen(LVGLDisplay* lvglDisplay)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _raceRunningScreen(nullptr)
    , _durationLabel(nullptr)
    , _durationValue(nullptr)
    , _btnDurationMinus(nullptr)
    , _btnDurationPlus(nullptr)
    , _btnStart(nullptr)
    , _raceDuration(60) {  // Default: 60 minutes
}

LVGLRaceSetupScreen::~LVGLRaceSetupScreen() {
}

void LVGLRaceSetupScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLRaceSetup] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLRaceSetup] Creating race setup screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header with back button (no umlauts - LVGL default font doesn't support them)
    createHeader("Race Setup", true, backBtnEventHandler, this);
    
    // Duration controls
    int y = HEADER_HEIGHT + Spacing::MD;
    _durationLabel = createLabel("Duration:", Spacing::MD, y, 100, 30, rgb565ToLVGL(Colors::TEXT));
    if (!_durationLabel) {
        Serial.println("[LVGLRaceSetup] ERROR: Failed to create duration label");
        return;
    }
    y += 35;
    
    // Duration value (larger text) - center it
    _durationValue = createLabel("60 Min", 0, y, SCREEN_WIDTH, 40, rgb565ToLVGL(Colors::TEXT));
    if (!_durationValue) {
        Serial.println("[LVGLRaceSetup] ERROR: Failed to create duration value label");
        return;
    }
    lv_obj_set_style_text_align(_durationValue, LV_TEXT_ALIGN_CENTER, 0);
    
    // Duration buttons
    int btnW = 60;
    int btnH = 50;
    _btnDurationMinus = createButton("-", Spacing::MD, y, btnW, btnH, durationMinusEventHandler, this);
    if (!_btnDurationMinus) {
        Serial.println("[LVGLRaceSetup] ERROR: Failed to create minus button");
        return;
    }
    
    _btnDurationPlus = createButton("+", SCREEN_WIDTH - Spacing::MD - btnW, y, btnW, btnH, durationPlusEventHandler, this);
    if (!_btnDurationPlus) {
        Serial.println("[LVGLRaceSetup] ERROR: Failed to create plus button");
        return;
    }
    
    y += 70;
    
    // Start button
    _btnStart = createButton("START", Spacing::MD, y, SCREEN_WIDTH - 2 * Spacing::MD, BUTTON_HEIGHT,
                            startBtnEventHandler, this);
    if (!_btnStart) {
        Serial.println("[LVGLRaceSetup] ERROR: Failed to create start button");
        return;
    }
    lv_obj_set_style_bg_color(_btnStart, rgb565ToLVGL(Colors::SECONDARY), 0);
    
    updateDurationDisplay();
    
    Serial.println("[LVGLRaceSetup] Race setup screen created");
}

void LVGLRaceSetupScreen::onExit() {
    // Cleanup if needed
}

void LVGLRaceSetupScreen::updateDurationDisplay() {
    if (!_durationValue) {
        Serial.println("[LVGLRaceSetup] WARNING: _durationValue is null in updateDurationDisplay");
        return;
    }
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%lu Min", _raceDuration);
    lv_label_set_text(_durationValue, buf);
}

void LVGLRaceSetupScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLRaceSetup] Back button clicked");
        screen->_navigation->goBack();
    }
}

void LVGLRaceSetupScreen::durationMinusEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_durationValue) {
        Serial.println("[LVGLRaceSetup] ERROR: screen or durationValue is null in durationMinusEventHandler");
        return;
    }
    
    if (screen->_raceDuration > 5) {
        screen->_raceDuration -= 5;
        screen->updateDurationDisplay();
    }
}

void LVGLRaceSetupScreen::durationPlusEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_durationValue) {
        Serial.println("[LVGLRaceSetup] ERROR: screen or durationValue is null in durationPlusEventHandler");
        return;
    }
    
    if (screen->_raceDuration < 120) {
        screen->_raceDuration += 5;
        screen->updateDurationDisplay();
    }
}

void LVGLRaceSetupScreen::startBtnEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (!screen) {
        Serial.println("[LVGLRaceSetup] ERROR: screen is null");
        return;
    }
    
    if (!screen->_navigation) {
        Serial.println("[LVGLRaceSetup] ERROR: navigation is null");
        return;
    }
    
    if (!screen->_raceRunningScreen) {
        Serial.println("[LVGLRaceSetup] ERROR: raceRunningScreen is null");
        return;
    }
    
    Serial.printf("[LVGLRaceSetup] Start button clicked - Duration: %lu minutes\n", screen->_raceDuration);
    Serial.printf("[LVGLRaceSetup] Navigation: %p, RaceRunningScreen: %p\n", 
                  screen->_navigation, screen->_raceRunningScreen);
    
    // Start race first (but don't call onEnter yet - navigation will do that)
    // Just set the race parameters
    screen->_raceRunningScreen->startRace(screen->_raceDuration);
    
    // Small delay to ensure race state is set
    delay(10);
    
    // Navigate to race running screen
    // This will call onEnter() which will create the UI
    Serial.println("[LVGLRaceSetup] Navigating to race running screen...");
    screen->_navigation->setScreen(screen->_raceRunningScreen);
    Serial.println("[LVGLRaceSetup] Navigation completed");
}

