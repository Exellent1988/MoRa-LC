#include "lvgl_team_edit_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLTeamEditScreen::LVGLTeamEditScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _lapCounter(lapCounter)
    , _teamId(0)
    , _nameTextArea(nullptr)
    , _keyboard(nullptr) {
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
    int y = HEADER_HEIGHT + Spacing::SM;
    createLabel("Team Name:", Spacing::MD, y, 150, 20, rgb565ToLVGL(Colors::TEXT));
    y += 24;
    
    // Text area for team name
    _nameTextArea = lv_textarea_create(_screen);
    lv_obj_set_size(_nameTextArea, SCREEN_WIDTH - 2 * Spacing::MD, 35);
    lv_obj_set_pos(_nameTextArea, Spacing::MD, y);
    lv_textarea_set_placeholder_text(_nameTextArea, "Enter team name");
    lv_textarea_set_one_line(_nameTextArea, true);
    lv_obj_set_style_bg_color(_nameTextArea, rgb565ToLVGL(Colors::SURFACE), 0);
    lv_obj_set_style_text_color(_nameTextArea, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_style_border_width(_nameTextArea, 1, 0);
    lv_obj_set_style_border_color(_nameTextArea, rgb565ToLVGL(Colors::BORDER), 0);
    
    // If editing existing team, load name
    if (_teamId > 0 && _lapCounter) {
        TeamData* team = _lapCounter->getTeam(_teamId);
        if (team) {
            lv_textarea_set_text(_nameTextArea, team->teamName.c_str());
        }
    }
    
    y += 39;
    
    // Keyboard fills remaining space (uses built-in OK/Cancel keys)
    _keyboard = lv_keyboard_create(_screen);
    lv_keyboard_set_textarea(_keyboard, _nameTextArea);
    lv_keyboard_set_mode(_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_set_size(_keyboard, SCREEN_WIDTH, SCREEN_HEIGHT - y);
    lv_obj_set_pos(_keyboard, 0, y);
    lv_obj_set_style_bg_color(_keyboard, rgb565ToLVGL(Colors::BACKGROUND), 0);
    lv_obj_add_event_cb(_keyboard, keyboardEventHandler, LV_EVENT_READY, this);
    lv_obj_add_event_cb(_keyboard, keyboardEventHandler, LV_EVENT_CANCEL, this);
    
    Serial.println("[LVGLTeamEdit] Team edit screen created");
}

void LVGLTeamEditScreen::onExit() {
    // Hide keyboard when leaving screen
    if (_keyboard) {
        lv_obj_add_flag(_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

void LVGLTeamEditScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLTeamEditScreen* screen = (LVGLTeamEditScreen*)lv_event_get_user_data(e);
    if (screen) {
        // Hide keyboard before navigating back
        if (screen->_keyboard) {
            lv_obj_add_flag(screen->_keyboard, LV_OBJ_FLAG_HIDDEN);
        }
        if (screen->_navigation) {
            Serial.println("[LVGLTeamEdit] Back button clicked");
            screen->_navigation->goBack();
        }
    }
}

void LVGLTeamEditScreen::keyboardEventHandler(lv_event_t* e) {
    LVGLTeamEditScreen* screen = (LVGLTeamEditScreen*)lv_event_get_user_data(e);
    if (!screen) return;
    
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_READY) {
        // OK key pressed - save
        if (!screen->_lapCounter || !screen->_nameTextArea) return;
        
        const char* name = lv_textarea_get_text(screen->_nameTextArea);
        if (!name || strlen(name) == 0) {
            Serial.println("[LVGLTeamEdit] Team name is empty");
            return;
        }
        
        if (screen->_teamId == 0) {
            if (screen->_lapCounter->addTeam(String(name))) {
                Serial.printf("[LVGLTeamEdit] Team added: %s\n", name);
                if (screen->_navigation) screen->_navigation->goBack();
            }
        } else {
            if (screen->_lapCounter->updateTeam(screen->_teamId, String(name))) {
                Serial.printf("[LVGLTeamEdit] Team updated: ID=%u, Name=%s\n", screen->_teamId, name);
                if (screen->_navigation) screen->_navigation->goBack();
            }
        }
    } else if (code == LV_EVENT_CANCEL) {
        // Cancel key pressed
        Serial.println("[LVGLTeamEdit] Cancel pressed");
        if (screen->_navigation) screen->_navigation->goBack();
    }
}
