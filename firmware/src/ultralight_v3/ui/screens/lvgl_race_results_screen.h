#ifndef LVGL_RACE_RESULTS_SCREEN_H
#define LVGL_RACE_RESULTS_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/lap_counter_service.h"
#include <lvgl.h>

// Forward declaration
class LVGLNavigation;

/**
 * Race Results Screen (LVGL8)
 * Display race results and historical data
 */
class LVGLRaceResultsScreen : public LVGLBaseScreen {
public:
    LVGLRaceResultsScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter);
    virtual ~LVGLRaceResultsScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    // Set navigation for back button
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
private:
    LVGLNavigation* _navigation;
    LapCounterService* _lapCounter;
    lv_obj_t* _list;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    
    // Helper methods
    void updateResultsList();
};

#endif // LVGL_RACE_RESULTS_SCREEN_H

