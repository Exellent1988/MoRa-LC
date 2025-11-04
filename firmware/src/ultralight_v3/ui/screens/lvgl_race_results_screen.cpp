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
    
    // Create header with back button (ASCII only)
    createHeader("Results", true, backBtnEventHandler, this);
    
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
        lv_obj_t* emptyItem = lv_list_add_btn(_list, LV_SYMBOL_FILE, "LapCounter not available");
        styleListItem(emptyItem, Fonts::Size::Body);
        return;
    }
    
    std::vector<TeamData*> leaderboard = _lapCounter->getLeaderboard();
    
    if (leaderboard.empty()) {
        lv_obj_t* emptyItem = lv_list_add_btn(_list, LV_SYMBOL_FILE, "No results");
        styleListItem(emptyItem, Fonts::Size::Body);
        return;
    }
    
    // Show results sorted by lap count (ASCII only)
    int rank = 1;
    for (TeamData* team : leaderboard) {
        char info[128];
        if (team->bestLapTime > 0) {
            uint32_t totalMillis = team->bestLapTime;
            uint32_t minutes = totalMillis / 60000;
            uint32_t seconds = (totalMillis % 60000) / 1000;
            uint32_t millis = totalMillis % 1000;
            snprintf(info, sizeof(info), "%u. %s\nLaps: %u | Best: %lu:%02lu.%03lu", 
                    rank, team->teamName.c_str(), team->lapCount, minutes, seconds, millis);
        } else {
            snprintf(info, sizeof(info), "%u. %s\nLaps: %u", 
                    rank, team->teamName.c_str(), team->lapCount);
        }
        
        lv_obj_t* item = lv_list_add_btn(_list, LV_SYMBOL_FILE, info);
        styleListItem(item, Fonts::Size::Body);

        if (rank == 1) {
            lv_color_t color = rgb565ToLVGL(Colors::PRIMARY);
            lv_obj_set_style_bg_color(item, color, LV_PART_MAIN);
            lv_obj_set_style_bg_color(item, color, LV_PART_MAIN | LV_STATE_PRESSED);
        } else if (rank == 2) {
            lv_color_t color = rgb565ToLVGL(Colors::SECONDARY);
            lv_obj_set_style_bg_color(item, color, LV_PART_MAIN);
            lv_obj_set_style_bg_color(item, color, LV_PART_MAIN | LV_STATE_PRESSED);
        } else if (rank == 3) {
            lv_color_t color = rgb565ToLVGL(Colors::ACCENT);
            lv_obj_set_style_bg_color(item, color, LV_PART_MAIN);
            lv_obj_set_style_bg_color(item, color, LV_PART_MAIN | LV_STATE_PRESSED);
        }
        
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

