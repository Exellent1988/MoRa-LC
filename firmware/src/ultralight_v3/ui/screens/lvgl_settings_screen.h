#ifndef LVGL_SETTINGS_SCREEN_H
#define LVGL_SETTINGS_SCREEN_H

#include "lvgl_base_screen.h"
#include <lvgl.h>

// Forward declaration
class LVGLNavigation;

/**
 * Settings Screen (LVGL8)
 * System settings and configuration
 */
class LVGLSettingsScreen : public LVGLBaseScreen {
public:
    LVGLSettingsScreen(LVGLDisplay* lvglDisplay);
    virtual ~LVGLSettingsScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    // Set navigation for back button
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
private:
    LVGLNavigation* _navigation;
    lv_obj_t* _list;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    
    // Helper methods
    void createSettingsList();
};

#endif // LVGL_SETTINGS_SCREEN_H

