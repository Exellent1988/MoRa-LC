#include "lvgl_teams_screen.h"
#include "lvgl_beacon_assign_screen.h"
#include "lvgl_team_edit_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLTeamsScreen::LVGLTeamsScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _lapCounter(lapCounter)
    , _beaconAssignScreen(nullptr)
    , _teamEditScreen(nullptr)
    , _list(nullptr)
    , _addButton(nullptr) {
}

LVGLTeamsScreen::~LVGLTeamsScreen() {
}

void LVGLTeamsScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLTeams] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLTeams] Creating teams screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header with back button
    createHeader("Teams", true, backBtnEventHandler, this);
    
    // Create list for teams
    int listY = HEADER_HEIGHT + Spacing::SM;
    int listH = SCREEN_HEIGHT - listY - BUTTON_HEIGHT - Spacing::MD;
    _list = createList(Spacing::MD, listY, SCREEN_WIDTH - 2 * Spacing::MD, listH);
    
    // Create add button
    int btnY = SCREEN_HEIGHT - BUTTON_HEIGHT - Spacing::MD;
    _addButton = createButton("+ Neues Team", Spacing::MD, btnY, 
                             SCREEN_WIDTH - 2 * Spacing::MD, BUTTON_HEIGHT,
                             addBtnEventHandler, this);
    lv_obj_set_style_bg_color(_addButton, rgb565ToLVGL(Colors::SECONDARY), 0);
    
    // Update team list
    updateTeamList();
    
    Serial.println("[LVGLTeams] Teams screen created");
}

void LVGLTeamsScreen::onExit() {
    // Cleanup if needed
}

void LVGLTeamsScreen::updateTeamList() {
    if (!_list) return;
    
    // Clear existing list items
    lv_obj_clean(_list);
    
    if (!_lapCounter) {
        lv_obj_t* emptyItem = lv_list_add_btn(_list, LV_SYMBOL_FILE, "LapCounter nicht verfügbar");
        lv_obj_set_style_bg_color(emptyItem, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    std::vector<TeamData*> teams = _lapCounter->getAllTeams();
    
    if (teams.empty()) {
        lv_obj_t* emptyItem = lv_list_add_btn(_list, LV_SYMBOL_FILE, "Keine Teams");
        lv_obj_set_style_bg_color(emptyItem, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    // Add teams to list
    for (TeamData* team : teams) {
        char info[128];
        if (team->beaconUUID.length() > 0) {
            snprintf(info, sizeof(info), "%u. %s\nBeacon: %.12s", 
                    team->teamId, team->teamName.c_str(), team->beaconUUID.c_str());
        } else {
            snprintf(info, sizeof(info), "%u. %s\nKein Beacon", 
                    team->teamId, team->teamName.c_str());
        }
        
        lv_obj_t* item = lv_list_add_btn(_list, LV_SYMBOL_FILE, info);
        lv_obj_add_event_cb(item, teamItemEventHandler, LV_EVENT_CLICKED, this);
        
        // Store team ID in user data (we'll use a simple approach)
        // For now, team ID is in the text
    }
}

void LVGLTeamsScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLTeamsScreen* screen = (LVGLTeamsScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLTeams] Back button clicked");
        screen->_navigation->goBack();
    }
}

void LVGLTeamsScreen::addBtnEventHandler(lv_event_t* e) {
    LVGLTeamsScreen* screen = (LVGLTeamsScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_navigation || !screen->_teamEditScreen) {
        Serial.println("[LVGLTeams] ERROR: Cannot add team - navigation or teamEditScreen is null");
        return;
    }
    
    Serial.println("[LVGLTeams] Add button clicked");
    screen->_teamEditScreen->setTeam(0);  // 0 = new team
    screen->_navigation->setScreen(screen->_teamEditScreen);
}

void LVGLTeamsScreen::teamItemEventHandler(lv_event_t* e) {
    LVGLTeamsScreen* screen = (LVGLTeamsScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_navigation || !screen->_beaconAssignScreen) return;
    
    // Extract team ID from label text (first number before ".")
    lv_obj_t* item = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(item, 1);
    if (label) {
        const char* text = lv_label_get_text(label);
        uint8_t teamId = atoi(text); // Extract first number
        
        if (teamId > 0) {
            screen->_beaconAssignScreen->setTeam(teamId);
            screen->_navigation->setScreen(screen->_beaconAssignScreen);
        }
    }
}

