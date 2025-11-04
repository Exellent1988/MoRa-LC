#ifndef LVGL_BEACON_ASSIGN_SCREEN_H
#define LVGL_BEACON_ASSIGN_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/beacon_service.h"
#include "../../services/lap_counter_service.h"
#include <lvgl.h>

// Forward declaration
class LVGLNavigation;

/**
 * Beacon Assignment Screen (LVGL8)
 * Assign beacons to teams
 */
class LVGLBeaconAssignScreen : public LVGLBaseScreen {
public:
    LVGLBeaconAssignScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService, LapCounterService* lapCounter);
    virtual ~LVGLBeaconAssignScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override;
    
    // Set navigation
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
    // Set team to assign beacon to
    void setTeam(uint8_t teamId) { _teamId = teamId; }
    
private:
    LVGLNavigation* _navigation;
    BeaconService* _beaconService;
    LapCounterService* _lapCounter;
    uint8_t _teamId;
    
    lv_obj_t* _instructionLabel;
    lv_obj_t* _beaconList;
    uint32_t _lastUpdate;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    static void beaconItemEventHandler(lv_event_t* e);
    
    // Helper methods
    void updateBeaconList();
    void assignBeaconToTeam(const String& beaconUUID);
};

#endif // LVGL_BEACON_ASSIGN_SCREEN_H

