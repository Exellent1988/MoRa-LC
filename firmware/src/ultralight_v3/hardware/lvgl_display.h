#ifndef LVGL_DISPLAY_H
#define LVGL_DISPLAY_H

#include <Arduino.h>
#include <lvgl.h>
#include "../core/config.h"
#include "display.h"

/**
 * LVGL8 Display Wrapper
 * Integrates LVGL8 with LovyanGFX as display driver
 * Based on ESP32-Cheap-Yellow-Display example
 */
class LVGLDisplay {
public:
    LVGLDisplay(Display* display);
    ~LVGLDisplay();
    
    // Initialization
    bool begin();
    bool isReady() const { return _initialized; }
    
    // LVGL Access
    lv_disp_t* getDisplay() { return _lvgl_display; }  // LVGL8 uses lv_disp_t
    lv_obj_t* getScreen() { return lv_scr_act(); }
    
    // Update (call in loop)
    void update();
    
private:
    Display* _display;
    lv_disp_t* _lvgl_display;  // LVGL8 uses lv_disp_t
    bool _initialized;
    
    // LVGL callbacks (LVGL8 API)
    static void flushCallback(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p);
    static void touchpadReadCallback(lv_indev_drv_t* indev_drv, lv_indev_data_t* data);
    
    
    // Static instance for callbacks
    static LVGLDisplay* _instance;
};

#endif // LVGL_DISPLAY_H

