#ifndef LVGL_RACE_SETUP_SCREEN_H
#define LVGL_RACE_SETUP_SCREEN_H

#include "lvgl_base_screen.h"
#include <lvgl.h>

class LVGLNavigation;
class LVGLRaceRunningScreen;

class LVGLRaceSetupScreen : public LVGLBaseScreen {
public:
    LVGLRaceSetupScreen(LVGLDisplay* lvglDisplay);
    virtual ~LVGLRaceSetupScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    void setRaceRunningScreen(LVGLRaceRunningScreen* screen) { _raceRunningScreen = screen; }
    
    const char* getRaceName() const;
    
private:
    LVGLNavigation* _navigation;
    LVGLRaceRunningScreen* _raceRunningScreen;
    
    // Controls container (hidden when keyboard is visible)
    lv_obj_t* _controlsContainer;
    lv_obj_t* _nameTextArea;
    lv_obj_t* _durationValue;
    lv_obj_t* _btnStart;
    lv_obj_t* _keyboard;
    
    uint32_t _raceDuration;
    bool _keyboardVisible;
    
    static void backBtnEventHandler(lv_event_t* e);
    static void durationMinusEventHandler(lv_event_t* e);
    static void durationPlusEventHandler(lv_event_t* e);
    static void durationBigPlusEventHandler(lv_event_t* e);
    static void startBtnEventHandler(lv_event_t* e);
    static void nameTextAreaEventHandler(lv_event_t* e);
    static void keyboardEventHandler(lv_event_t* e);
    
    void updateDurationDisplay();
    void showKeyboard();
    void hideKeyboard();
};

#endif // LVGL_RACE_SETUP_SCREEN_H
