#ifndef LVGL_BASE_SCREEN_H
#define LVGL_BASE_SCREEN_H

#include <lvgl.h>
#include "../../hardware/lvgl_display.h"
#include "../theme.h"

/**
 * Base Screen Class for LVGL8
 * All LVGL screens inherit from this
 */
class LVGLBaseScreen {
public:
    LVGLBaseScreen(LVGLDisplay* lvglDisplay);
    virtual ~LVGLBaseScreen();
    
    // Core screen methods
    virtual void onEnter() = 0;
    virtual void onExit() {}
    virtual void update() {}  // Called in loop for dynamic updates
    
    // Access
    lv_obj_t* getScreen() { return _screen; }
    LVGLDisplay* getLVGLDisplay() { return _lvglDisplay; }
    
protected:
    LVGLDisplay* _lvglDisplay;
    lv_obj_t* _screen;
    
    // Helper methods for common UI elements
    lv_obj_t* createHeader(const char* title, bool showBack = false, lv_event_cb_t backHandler = nullptr, void* user_data = nullptr);
    lv_obj_t* createButton(const char* label, int x, int y, int w, int h, lv_event_cb_t event_cb, void* user_data = nullptr);
    lv_obj_t* createLabel(const char* text, int x, int y, int w, int h,
                          lv_color_t color = lv_color_hex(0x000000),
                          Fonts::Size fontSize = Fonts::Size::Body,
                          lv_text_align_t align = LV_TEXT_ALIGN_LEFT);
    lv_obj_t* createList(int x, int y, int w, int h);
    void styleListItem(lv_obj_t* item, Fonts::Size fontSize = Fonts::Size::Body);
    
    // Color conversion helpers
    static lv_color_t rgb565ToLVGL(uint16_t rgb565);
    static lv_color_t hexToLVGL(uint32_t hex);
};

#endif // LVGL_BASE_SCREEN_H

