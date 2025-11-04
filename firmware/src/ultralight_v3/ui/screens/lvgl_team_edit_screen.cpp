#include "lvgl_team_edit_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLTeamEditScreen::LVGLTeamEditScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _lapCounter(lapCounter)
    , _teamId(0)
    , _nameTextArea(nullptr)
    , _saveButton(nullptr)
    , _cancelButton(nullptr) {
}

LVGLTeamEditScreen::~LVGLTeamEditScreen() {
}

void LVGLTeamEditScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLTeamEdit] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLTeamEdit] Creating team edit screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    if (!_screen) {
        Serial.println("[LVGLTeamEdit] ERROR: Could not get screen");
        return;
    }
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header
    createHeader(_teamId == 0 ? "New Team" : "Edit Team", true, backBtnEventHandler, this);
    
    // Name label
    int y = HEADER_HEIGHT + Spacing::MD;
    createLabel("Team Name:", Spacing::MD, y, 150, 25, rgb565ToLVGL(Colors::TEXT));
    y += 30;
    
    // Text area for team name
    _nameTextArea = lv_textarea_create(_screen);
    lv_obj_set_size(_nameTextArea, SCREEN_WIDTH - 2 * Spacing::MD, 40);
    lv_obj_set_pos(_nameTextArea, Spacing::MD, y);
    lv_textarea_set_placeholder_text(_nameTextArea, "Enter team name");
    lv_obj_set_style_bg_color(_nameTextArea, rgb565ToLVGL(Colors::SURFACE), 0);
    lv_obj_set_style_text_color(_nameTextArea, rgb565ToLVGL(Colors::TEXT), 0);
    
    // If editing existing team, load name
    if (_teamId > 0 && _lapCounter) {
        TeamData* team = _lapCounter->getTeam(_teamId);
        if (team) {
            lv_textarea_set_text(_nameTextArea, team->teamName.c_str());
        }
    }
    
    y += 60;
    
    // Buttons
    int btnW = (SCREEN_WIDTH - 3 * Spacing::MD) / 2;
    int btnH = BUTTON_HEIGHT;
    
    _saveButton = createButton("Save", Spacing::MD, y, btnW, btnH, saveBtnEventHandler, this);
    lv_obj_set_style_bg_color(_saveButton, rgb565ToLVGL(Colors::SECONDARY), 0);
    
    _cancelButton = createButton("Cancel", Spacing::MD * 2 + btnW, y, btnW, btnH, cancelBtnEventHandler, this);
    
    Serial.println("[LVGLTeamEdit] Team edit screen created");
}

void LVGLTeamEditScreen::onExit() {
    // Cleanup if needed
}

void LVGLTeamEditScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLTeamEditScreen* screen = (LVGLTeamEditScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLTeamEdit] Back button clicked");
        screen->_navigation->goBack();
    }
}

void LVGLTeamEditScreen::saveBtnEventHandler(lv_event_t* e) {
    LVGLTeamEditScreen* screen = (LVGLTeamEditScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_lapCounter || !screen->_nameTextArea) {
        Serial.println("[LVGLTeamEdit] ERROR: Cannot save - missing dependencies");
        return;
    }
    
    const char* name = lv_textarea_get_text(screen->_nameTextArea);
    if (!name || strlen(name) == 0) {
        Serial.println("[LVGLTeamEdit] ERROR: Team name is empty");
        return;
    }
    
    if (screen->_teamId == 0) {
        // New team
        if (screen->_lapCounter->addTeam(String(name))) {
            Serial.printf("[LVGLTeamEdit] Team added: %s\n", name);
            // Teams are saved automatically or manually via Settings
            if (screen->_navigation) {
                screen->_navigation->goBack();
            }
        } else {
            Serial.println("[LVGLTeamEdit] ERROR: Failed to add team");
        }
    } else {
        // Update existing team
        if (screen->_lapCounter->updateTeam(screen->_teamId, String(name))) {
            Serial.printf("[LVGLTeamEdit] Team updated: ID=%u, Name=%s\n", screen->_teamId, name);
            // Teams are saved automatically or manually via Settings
            if (screen->_navigation) {
                screen->_navigation->goBack();
            }
        } else {
            Serial.println("[LVGLTeamEdit] ERROR: Failed to update team");
        }
    }
}

void LVGLTeamEditScreen::cancelBtnEventHandler(lv_event_t* e) {
    LVGLTeamEditScreen* screen = (LVGLTeamEditScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLTeamEdit] Cancel button clicked");
        screen->_navigation->goBack();
    }
}
