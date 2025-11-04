#ifndef LVGL_RACE_SETUP_SCREEN_H
#define LVGL_RACE_SETUP_SCREEN_H

#include "lvgl_base_screen.h"
#include <lvgl.h>

// Forward declarations
class LVGLNavigation;
class LVGLRaceRunningScreen;

/**
 * Race Setup Screen (LVGL8)
 * Configure race settings before starting
 */
class LVGLRaceSetupScreen : public LVGLBaseScreen {
public:
    LVGLRaceSetupScreen(LVGLDisplay* lvglDisplay);
    virtual ~LVGLRaceSetupScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    // Set navigation for back button
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
    // Set race running screen for navigation
    void setRaceRunningScreen(LVGLRaceRunningScreen* screen) { _raceRunningScreen = screen; }
    
private:
    LVGLNavigation* _navigation;
    LVGLRaceRunningScreen* _raceRunningScreen;
    lv_obj_t* _durationLabel;
    lv_obj_t* _durationValue;
    lv_obj_t* _btnDurationMinus;
    lv_obj_t* _btnDurationPlus;
    lv_obj_t* _btnStart;
    
    uint32_t _raceDuration;  // in minutes
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    static void durationMinusEventHandler(lv_event_t* e);
    static void durationPlusEventHandler(lv_event_t* e);
    static void startBtnEventHandler(lv_event_t* e);
    
    // Helper methods
    void updateDurationDisplay();
};

#endif // LVGL_RACE_SETUP_SCREEN_H

