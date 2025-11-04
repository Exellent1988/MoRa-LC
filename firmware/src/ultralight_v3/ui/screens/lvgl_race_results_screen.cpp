#include "lvgl_race_results_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLRaceResultsScreen::LVGLRaceResultsScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _lapCounter(lapCounter)
    , _list(nullptr) {
}

LVGLRaceResultsScreen::~LVGLRaceResultsScreen() {
}

void LVGLRaceResultsScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLRaceResults] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLRaceResults] Creating race results screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header with back button
    createHeader("Ergebnisse", true, backBtnEventHandler, this);
    
    // Create list for results
    int listY = HEADER_HEIGHT + Spacing::SM;
    int listH = SCREEN_HEIGHT - listY - Spacing::MD;
    _list = createList(Spacing::MD, listY, SCREEN_WIDTH - 2 * Spacing::MD, listH);
    
    // Update results list
    updateResultsList();
    
    Serial.println("[LVGLRaceResults] Race results screen created");
}

void LVGLRaceResultsScreen::onExit() {
    // Cleanup if needed
}

void LVGLRaceResultsScreen::updateResultsList() {
    if (!_list) return;
    
    // Clear existing list items
    lv_obj_clean(_list);
    
    if (!_lapCounter) {
        lv_obj_t* emptyItem = lv_list_add_btn(_list, LV_SYMBOL_FILE, "LapCounter nicht verfügbar");
        lv_obj_set_style_bg_color(emptyItem, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    std::vector<TeamData*> leaderboard = _lapCounter->getLeaderboard();
    
    if (leaderboard.empty()) {
        lv_obj_t* emptyItem = lv_list_add_btn(_list, LV_SYMBOL_FILE, "Keine Ergebnisse");
        lv_obj_set_style_bg_color(emptyItem, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    // Show results sorted by lap count
    int rank = 1;
    for (TeamData* team : leaderboard) {
        char info[128];
        if (team->bestLapTime > 0) {
            uint32_t seconds = team->bestLapTime / 1000;
            uint32_t minutes = seconds / 60;
            seconds %= 60;
            snprintf(info, sizeof(info), "%u. %s\nRunden: %u | Beste Zeit: %lu:%02lu", 
                    rank, team->teamName.c_str(), team->lapCount, minutes, seconds);
        } else {
            snprintf(info, sizeof(info), "%u. %s\nRunden: %u", 
                    rank, team->teamName.c_str(), team->lapCount);
        }
        
        lv_obj_t* item = lv_list_add_btn(_list, LV_SYMBOL_FILE, info);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        
        rank++;
    }
}

void LVGLRaceResultsScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLRaceResultsScreen* screen = (LVGLRaceResultsScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLRaceResults] Back button clicked");
        screen->_navigation->goBack();
    }
}

