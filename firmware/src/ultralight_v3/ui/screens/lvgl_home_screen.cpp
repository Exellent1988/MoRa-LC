#include "lvgl_home_screen.h"
#include "../navigation_lvgl.h"
#include "../widgets/icons.h"
#include "lvgl_teams_screen.h"
#include "lvgl_race_setup_screen.h"
#include "lvgl_race_results_screen.h"
#include "lvgl_settings_screen.h"
#include <Arduino.h>

LVGLHomeScreen::LVGLHomeScreen(LVGLDisplay* lvglDisplay)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _teamsScreen(nullptr)
    , _raceSetupScreen(nullptr)
    , _raceResultsScreen(nullptr)
    , _settingsScreen(nullptr)
    , _btnRace(nullptr)
    , _btnTeams(nullptr)
    , _btnResults(nullptr)
    , _btnSettings(nullptr) {
}

LVGLHomeScreen::~LVGLHomeScreen() {
}

void LVGLHomeScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLHomeScreen] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLHomeScreen] Creating home screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header
    createHeader("MoRa-LC", false);
    
    // Calculate button positions (centered, vertical layout)
    int startY = HEADER_HEIGHT + Spacing::MD;
    int btnW = SCREEN_WIDTH - 2 * Spacing::MD;
    int btnH = BUTTON_HEIGHT;
    int btnSpacing = Spacing::SM;
    int x = Spacing::MD;
    
    // Race button with icon
    _btnRace = createButton("Neues Rennen", x, startY, btnW, btnH, btnRaceEventHandler, this);
    lv_obj_set_style_bg_color(_btnRace, rgb565ToLVGL(Colors::SECONDARY), 0);
    createIconImage(_btnRace, ICON_RACE, 24);
    startY += btnH + btnSpacing;
    
    // Teams button with icon
    _btnTeams = createButton("Teams", x, startY, btnW, btnH, btnTeamsEventHandler, this);
    createIconImage(_btnTeams, ICON_TEAM, 24);
    startY += btnH + btnSpacing;
    
    // Results button with icon
    _btnResults = createButton("Ergebnisse", x, startY, btnW, btnH, btnResultsEventHandler, this);
    createIconImage(_btnResults, ICON_RESULTS, 24);
    startY += btnH + btnSpacing;
    
    // Settings button with icon
    _btnSettings = createButton("Einstellungen", x, startY, btnW, btnH, btnSettingsEventHandler, this);
    createIconImage(_btnSettings, ICON_SETTINGS, 24);
    
    Serial.println("[LVGLHomeScreen] Home screen created");
}

void LVGLHomeScreen::onExit() {
    // Cleanup if needed
}

void LVGLHomeScreen::btnRaceEventHandler(lv_event_t* e) {
    LVGLHomeScreen* screen = (LVGLHomeScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation && screen->_raceSetupScreen) {
        Serial.println("[LVGLHomeScreen] Race button clicked");
        screen->_navigation->setScreen(screen->_raceSetupScreen);
    }
}

void LVGLHomeScreen::btnTeamsEventHandler(lv_event_t* e) {
    LVGLHomeScreen* screen = (LVGLHomeScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation && screen->_teamsScreen) {
        Serial.println("[LVGLHomeScreen] Teams button clicked");
        screen->_navigation->setScreen(screen->_teamsScreen);
    }
}

void LVGLHomeScreen::btnResultsEventHandler(lv_event_t* e) {
    LVGLHomeScreen* screen = (LVGLHomeScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation && screen->_raceResultsScreen) {
        Serial.println("[LVGLHomeScreen] Results button clicked");
        screen->_navigation->setScreen(screen->_raceResultsScreen);
    }
}

void LVGLHomeScreen::btnSettingsEventHandler(lv_event_t* e) {
    LVGLHomeScreen* screen = (LVGLHomeScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation && screen->_settingsScreen) {
        Serial.println("[LVGLHomeScreen] Settings button clicked");
        screen->_navigation->setScreen(screen->_settingsScreen);
    }
}

