#include "lvgl_base_screen.h"
#include "../widgets/icons.h"
#include <Arduino.h>

LVGLBaseScreen::LVGLBaseScreen(LVGLDisplay* lvglDisplay)
    : _lvglDisplay(lvglDisplay)
    , _screen(nullptr) {
}

LVGLBaseScreen::~LVGLBaseScreen() {
    // LVGL objects are cleaned up automatically when screen is deleted
}

lv_obj_t* LVGLBaseScreen::createHeader(const char* title, bool showBack, lv_event_cb_t backHandler, void* user_data) {
    if (!_screen) return nullptr;
    
    // Create header container
    lv_obj_t* header = lv_obj_create(_screen);
    lv_obj_set_size(header, SCREEN_WIDTH, HEADER_HEIGHT);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, rgb565ToLVGL(Colors::HEADER_BG), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    
    // Title label
    lv_obj_t* titleLabel = lv_label_create(header);
    lv_label_set_text(titleLabel, title);
    lv_obj_set_style_text_color(titleLabel, rgb565ToLVGL(Colors::HEADER_TEXT), 0);
    lv_obj_align(titleLabel, LV_ALIGN_CENTER, 0, 0);
    
    // Back button (if needed)
    if (showBack) {
        lv_obj_t* backBtn = lv_btn_create(header);
        lv_obj_set_size(backBtn, 40, HEADER_HEIGHT);
        lv_obj_set_pos(backBtn, Spacing::SM, 0);
        lv_obj_set_style_bg_color(backBtn, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(backBtn, LV_OPA_TRANSP, 0);
        
        lv_obj_t* backLabel = lv_label_create(backBtn);
        lv_label_set_text(backLabel, "<");
        lv_obj_set_style_text_color(backLabel, rgb565ToLVGL(Colors::HEADER_TEXT), 0);
        lv_obj_center(backLabel);
        
        // Add event handler if provided
        if (backHandler) {
            lv_obj_add_event_cb(backBtn, backHandler, LV_EVENT_CLICKED, user_data);
        }
    }
    
    return header;
}

lv_obj_t* LVGLBaseScreen::createButton(const char* label, int x, int y, int w, int h, lv_event_cb_t event_cb, void* user_data) {
    if (!_screen) return nullptr;
    
    lv_obj_t* btn = lv_btn_create(_screen);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, rgb565ToLVGL(Colors::BUTTON), 0);
    lv_obj_set_style_bg_color(btn, rgb565ToLVGL(Colors::BUTTON_PRESSED), LV_STATE_PRESSED);
    
    if (event_cb) {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, user_data);
    }
    
    if (label) {
        lv_obj_t* btnLabel = lv_label_create(btn);
        lv_label_set_text(btnLabel, label);
        lv_obj_set_style_text_color(btnLabel, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
        lv_obj_center(btnLabel);
    }
    
    return btn;
}

lv_obj_t* LVGLBaseScreen::createLabel(const char* text, int x, int y, int w, int h, lv_color_t color) {
    if (!_screen) return nullptr;
    
    lv_obj_t* label = lv_label_create(_screen);
    if (!label) return nullptr;
    
    lv_label_set_text(label, text);
    lv_obj_set_size(label, w, h);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
    
    // Use larger font for better readability (if available)
    // Note: Fonts must be enabled in lv_conf.h
    // For now, use default font which should be readable
    
    return label;
}

lv_obj_t* LVGLBaseScreen::createList(int x, int y, int w, int h) {
    if (!_screen) return nullptr;
    
    lv_obj_t* list = lv_list_create(_screen);
    lv_obj_set_size(list, w, h);
    lv_obj_set_pos(list, x, y);
    lv_obj_set_style_bg_color(list, rgb565ToLVGL(Colors::BACKGROUND), 0);
    
    return list;
}

lv_color_t LVGLBaseScreen::rgb565ToLVGL(uint16_t rgb565) {
    // Convert RGB565 to LVGL color
    // LVGL8 uses lv_color_t which is uint16_t when LV_COLOR_DEPTH=16
    // Direct cast works because both are RGB565 format
    lv_color_t color;
    color.full = rgb565;
    return color;
}

lv_color_t LVGLBaseScreen::hexToLVGL(uint32_t hex) {
    return lv_color_hex(hex);
}

