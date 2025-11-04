#include "lvgl_button.h"
#include "icons.h"
#include <Arduino.h>

LVGLButton::LVGLButton(lv_obj_t* parent, const char* label, int x, int y, int w, int h)
    : _obj(nullptr)
    , _label(nullptr)
    , _icon(nullptr) {
    
    _obj = lv_btn_create(parent);
    lv_obj_set_size(_obj, w, h);
    lv_obj_set_pos(_obj, x, y);
    lv_obj_set_style_bg_color(_obj, rgb565ToLVGL(Colors::BUTTON), 0);
    lv_obj_set_style_bg_color(_obj, rgb565ToLVGL(Colors::BUTTON_PRESSED), LV_STATE_PRESSED);
    
    if (label) {
        _label = lv_label_create(_obj);
        lv_label_set_text(_label, label);
        lv_obj_set_style_text_color(_label, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
        lv_obj_center(_label);
    }
}

LVGLButton::~LVGLButton() {
    // LVGL objects are cleaned up automatically
}

void LVGLButton::setColor(uint16_t color) {
    lv_obj_set_style_bg_color(_obj, rgb565ToLVGL(color), 0);
}

void LVGLButton::setTextColor(uint16_t color) {
    if (_label) {
        lv_obj_set_style_text_color(_label, rgb565ToLVGL(color), 0);
    }
}

void LVGLButton::setIcon(IconID iconId, uint8_t size) {
    // Remove existing icon
    if (_icon) {
        lv_obj_del(_icon);
        _icon = nullptr;
    }
    
    if (iconId != ICON_NONE) {
        _icon = createIconImage(_obj, iconId, size);
        if (_icon) {
            lv_obj_align(_icon, LV_ALIGN_LEFT_MID, 10, 0);
            // Adjust label position if icon exists
            if (_label) {
                lv_obj_align(_label, LV_ALIGN_CENTER, 10, 0);
            }
        }
    }
}

void LVGLButton::setCallback(lv_event_cb_t callback, void* user_data) {
    if (callback) {
        lv_obj_add_event_cb(_obj, callback, LV_EVENT_CLICKED, user_data);
    }
}

void LVGLButton::setEnabled(bool enabled) {
    if (enabled) {
        lv_obj_clear_state(_obj, LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(_obj, LV_STATE_DISABLED);
    }
}

bool LVGLButton::isEnabled() const {
    return !lv_obj_has_state(_obj, LV_STATE_DISABLED);
}

lv_color_t LVGLButton::rgb565ToLVGL(uint16_t rgb565) {
    lv_color_t color;
    color.full = rgb565;
    return color;
}

