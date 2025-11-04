#ifndef LVGL_BUTTON_H
#define LVGL_BUTTON_H

#include <lvgl.h>
#include "../theme.h"

/**
 * LVGL Button Widget
 * Wrapper for LVGL buttons with theme support
 */
class LVGLButton {
public:
    LVGLButton(lv_obj_t* parent, const char* label, int x, int y, int w, int h);
    ~LVGLButton();
    
    // Access
    lv_obj_t* getObj() { return _obj; }
    
    // Styling
    void setColor(uint16_t color);
    void setTextColor(uint16_t color);
    void setIcon(IconID iconId, uint8_t size = 16);
    
    // Events
    void setCallback(lv_event_cb_t callback, void* user_data = nullptr);
    
    // State
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
private:
    lv_obj_t* _obj;
    lv_obj_t* _label;
    lv_obj_t* _icon;
    
    static lv_color_t rgb565ToLVGL(uint16_t rgb565);
};

#endif // LVGL_BUTTON_H

