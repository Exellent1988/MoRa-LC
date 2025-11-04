#ifndef LVGL_TEAM_EDIT_SCREEN_H
#define LVGL_TEAM_EDIT_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/lap_counter_service.h"
#include <lvgl.h>

// Forward declaration
class LVGLNavigation;

/**
 * Team Edit Screen (LVGL8)
 * Add or edit team name
 */
class LVGLTeamEditScreen : public LVGLBaseScreen {
public:
    LVGLTeamEditScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter);
    virtual ~LVGLTeamEditScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    // Set navigation
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
    // Set team to edit (0 = new team)
    void setTeam(uint8_t teamId) { _teamId = teamId; }
    
private:
    LVGLNavigation* _navigation;
    LapCounterService* _lapCounter;
    uint8_t _teamId;
    
    lv_obj_t* _nameTextArea;
    lv_obj_t* _saveButton;
    lv_obj_t* _cancelButton;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    static void saveBtnEventHandler(lv_event_t* e);
    static void cancelBtnEventHandler(lv_event_t* e);
};

#endif // LVGL_TEAM_EDIT_SCREEN_H

