#ifndef LVGL_TEAMS_SCREEN_H
#define LVGL_TEAMS_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/lap_counter_service.h"
#include "../widgets/lvgl_dialog.h"
#include <lvgl.h>
#include <vector>

// Forward declarations
class LVGLNavigation;
class LVGLBeaconAssignScreen;
class LVGLTeamEditScreen;

/**
 * Teams Screen (LVGL8)
 * Manage teams list
 */
class LVGLTeamsScreen : public LVGLBaseScreen {
public:
    LVGLTeamsScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter);
    virtual ~LVGLTeamsScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    // Set navigation for back button
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
    // Set beacon assign screen
    void setBeaconAssignScreen(LVGLBeaconAssignScreen* screen) { _beaconAssignScreen = screen; }
    
    // Set team edit screen
    void setTeamEditScreen(LVGLTeamEditScreen* screen) { _teamEditScreen = screen; }
    
private:
    LVGLNavigation* _navigation;
    LapCounterService* _lapCounter;
    LVGLBeaconAssignScreen* _beaconAssignScreen;
    LVGLTeamEditScreen* _teamEditScreen;
    lv_obj_t* _list;
    lv_obj_t* _addButton;
    LVGLDialog* _dialog;
    uint8_t _pendingDeleteTeamId;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    static void addBtnEventHandler(lv_event_t* e);
    static void teamItemEventHandler(lv_event_t* e);
    static void deleteTeamEventHandler(lv_event_t* e);
    static void confirmDeleteHandler(lv_event_t* e);
    
    // Helper methods
    void updateTeamList();
    void showDeleteConfirmDialog(uint8_t teamId);
};

#endif // LVGL_TEAMS_SCREEN_H
