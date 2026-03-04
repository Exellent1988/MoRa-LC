#ifndef LVGL_DEBUG_SCREEN_H
#define LVGL_DEBUG_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/beacon_service.h"
#include "../../services/lap_counter_service.h"
#include <lvgl.h>

class LVGLNavigation;

class LVGLDebugScreen : public LVGLBaseScreen {
public:
    LVGLDebugScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService, LapCounterService* lapCounter);
    virtual ~LVGLDebugScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override;
    
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
private:
    LVGLNavigation* _navigation;
    BeaconService* _beaconService;
    LapCounterService* _lapCounter;
    lv_obj_t* _list;
    uint32_t _lastUpdate;
    
    static void backBtnEventHandler(lv_event_t* e);
    void refreshList();
};

#endif // LVGL_DEBUG_SCREEN_H
