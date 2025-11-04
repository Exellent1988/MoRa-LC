#ifndef LVGL_TEST_SCREEN_H
#define LVGL_TEST_SCREEN_H

#include "../../hardware/lvgl_display.h"
#include <lvgl.h>

/**
 * Simple LVGL Test Screen
 * Shows basic LVGL functionality
 */
class LVGLTestScreen {
public:
    LVGLTestScreen(LVGLDisplay* lvglDisplay);
    ~LVGLTestScreen();
    
    void onEnter();
    void update();
    
private:
    LVGLDisplay* _lvglDisplay;
    lv_obj_t* _screen;
    lv_obj_t* _labelTitle;
    lv_obj_t* _labelStatus;
    lv_obj_t* _btnTest;
    
    static void btnEventHandler(lv_event_t* e);
};

#endif // LVGL_TEST_SCREEN_H

