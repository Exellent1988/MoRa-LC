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
    
    // Create header with back button (ASCII only)
    createHeader("Teams", true, backBtnEventHandler, this);
    
    // Create list for teams
    int listY = HEADER_HEIGHT + Spacing::SM;
    int listH = SCREEN_HEIGHT - listY - BUTTON_HEIGHT - Spacing::MD;
    _list = createList(Spacing::MD, listY, SCREEN_WIDTH - 2 * Spacing::MD, listH);
    
    // Create add button (ASCII only - no umlauts)
    int btnY = SCREEN_HEIGHT - BUTTON_HEIGHT - Spacing::MD;
    _addButton = createButton("+ New Team", Spacing::MD, btnY, 
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
        lv_obj_t* emptyItem = lv_list_add_btn(_list, LV_SYMBOL_FILE, "LapCounter not available");
        lv_obj_set_style_bg_color(emptyItem, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    std::vector<TeamData*> teams = _lapCounter->getAllTeams();
    
    if (teams.empty()) {
        lv_obj_t* emptyItem = lv_list_add_btn(_list, LV_SYMBOL_FILE, "No teams");
        lv_obj_set_style_bg_color(emptyItem, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    // Add teams to list (ASCII only)
    for (TeamData* team : teams) {
        char info[128];
        if (team->beaconUUID.length() > 0) {
            snprintf(info, sizeof(info), "%u. %s\nBeacon: %.12s", 
                    team->teamId, team->teamName.c_str(), team->beaconUUID.c_str());
        } else {
            snprintf(info, sizeof(info), "%u. %s\nNo Beacon", 
                    team->teamId, team->teamName.c_str());
        }
        
        lv_obj_t* item = lv_list_add_btn(_list, LV_SYMBOL_FILE, info);
        
        // Store team ID in user data
        lv_obj_set_user_data(item, (void*)(uintptr_t)team->teamId);
        
        // Click: Navigate to beacon assign
        // Long press: Delete team (we'll add this later with a dialog)
        lv_obj_add_event_cb(item, teamItemEventHandler, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(item, deleteTeamEventHandler, LV_EVENT_LONG_PRESSED, this);
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
    if (!screen || !screen->_navigation) return;
    
    lv_obj_t* item = lv_event_get_target(e);
    uint8_t teamId = (uint8_t)(uintptr_t)lv_obj_get_user_data(item);
    
    if (teamId > 0) {
        // Navigate to beacon assign screen
        if (screen->_beaconAssignScreen) {
            screen->_beaconAssignScreen->setTeam(teamId);
            screen->_navigation->setScreen(screen->_beaconAssignScreen);
        }
    }
}

void LVGLTeamsScreen::deleteTeamEventHandler(lv_event_t* e) {
    LVGLTeamsScreen* screen = (LVGLTeamsScreen*)lv_event_get_user_data(e);
    if (!screen) return;
    
    lv_obj_t* item = lv_event_get_target(e);
    uint8_t teamId = (uint8_t)(uintptr_t)lv_obj_get_user_data(item);
    
    if (teamId > 0) {
        screen->showDeleteConfirmDialog(teamId);
    }
}

void LVGLTeamsScreen::showDeleteConfirmDialog(uint8_t teamId) {
    if (!_lapCounter) return;
    
    TeamData* team = _lapCounter->getTeam(teamId);
    if (!team) return;
    
    Serial.printf("[LVGLTeams] Showing delete confirmation for team %u: %s\n", teamId, team->teamName.c_str());
    
    // Simple confirmation - delete directly for now
    // TODO: Add proper LVGL dialog
    if (_lapCounter->removeTeam(teamId)) {
        Serial.printf("[LVGLTeams] Team %u deleted\n", teamId);
        updateTeamList();
    }
}

