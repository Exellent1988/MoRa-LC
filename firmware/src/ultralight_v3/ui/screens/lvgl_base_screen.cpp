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
    lv_obj_set_style_bg_color(header, rgb565ToLVGL(Colors::HEADER_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, Spacing::SM, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 0, LV_PART_MAIN);
    
    // Title label
    lv_obj_t* titleLabel = lv_label_create(header);
    lv_label_set_text(titleLabel, title);
    lv_obj_set_style_text_color(titleLabel, rgb565ToLVGL(Colors::HEADER_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(titleLabel, Fonts::get(Fonts::Size::Title), LV_PART_MAIN);
    lv_obj_set_style_text_align(titleLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(titleLabel, SCREEN_WIDTH - (showBack ? 2 * (Spacing::SM + 40) : 2 * Spacing::SM));
    lv_obj_align(titleLabel, LV_ALIGN_CENTER, 0, 0);
    
    // Back button (if needed)
    if (showBack) {
        lv_obj_t* backBtn = lv_btn_create(header);
        lv_obj_set_size(backBtn, 40, HEADER_HEIGHT);
        lv_obj_set_pos(backBtn, Spacing::SM, 0);
        lv_obj_set_style_bg_opa(backBtn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(backBtn, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(backBtn, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(backBtn, 0, LV_PART_MAIN);
        
        lv_obj_t* backLabel = lv_label_create(backBtn);
        lv_label_set_text(backLabel, "<");
        lv_obj_set_style_text_color(backLabel, rgb565ToLVGL(Colors::HEADER_TEXT), LV_PART_MAIN);
        lv_obj_set_style_text_font(backLabel, Fonts::get(Fonts::Size::Subtitle), LV_PART_MAIN);
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
    lv_obj_set_style_bg_color(btn, rgb565ToLVGL(Colors::BUTTON), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, rgb565ToLVGL(Colors::BUTTON_PRESSED), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, Spacing::SM, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn, Spacing::SM, LV_PART_MAIN);
    
    if (event_cb) {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, user_data);
    }
    
    if (label) {
        lv_obj_t* btnLabel = lv_label_create(btn);
        lv_label_set_text(btnLabel, label);
        lv_obj_set_style_text_color(btnLabel, rgb565ToLVGL(Colors::BUTTON_TEXT), LV_PART_MAIN);
        lv_obj_set_style_text_font(btnLabel, Fonts::get(Fonts::Size::Subtitle), LV_PART_MAIN);
        lv_obj_set_style_text_align(btnLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_center(btnLabel);
    }
    
    return btn;
}

lv_obj_t* LVGLBaseScreen::createLabel(const char* text, int x, int y, int w, int h,
                                      lv_color_t color, Fonts::Size fontSize, lv_text_align_t align) {
    if (!_screen) return nullptr;
    
    lv_obj_t* label = lv_label_create(_screen);
    if (!label) return nullptr;
    
    lv_label_set_text(label, text);
    lv_obj_set_size(label, w, h);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, Fonts::get(fontSize), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, align, LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_pad_all(label, 0, LV_PART_MAIN);
    
    return label;
}

lv_obj_t* LVGLBaseScreen::createList(int x, int y, int w, int h) {
    if (!_screen) return nullptr;
    
    lv_obj_t* list = lv_list_create(_screen);
    lv_obj_set_size(list, w, h);
    lv_obj_set_pos(list, x, y);
    lv_obj_set_style_bg_color(list, rgb565ToLVGL(Colors::SURFACE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(list, rgb565ToLVGL(Colors::BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(list, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(list, Spacing::SM, LV_PART_MAIN);
    lv_obj_set_style_pad_all(list, Spacing::SM / 2, LV_PART_MAIN);
    lv_obj_set_style_pad_row(list, Spacing::SM / 2, LV_PART_MAIN);
    lv_obj_set_style_pad_column(list, 0, LV_PART_MAIN);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(list, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_style_pad_top(list, 0, LV_PART_SCROLLBAR);
    
    return list;
}

void LVGLBaseScreen::styleListItem(lv_obj_t* item, Fonts::Size fontSize) {
    if (!item) return;

    lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::LIST_ITEM), LV_PART_MAIN);
    lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::LIST_ITEM_SELECTED), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::LIST_ITEM_SELECTED), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(item, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(item, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(item, Spacing::SM / 2, LV_PART_MAIN);
    lv_obj_set_style_pad_left(item, Spacing::SM, LV_PART_MAIN);
    lv_obj_set_style_pad_right(item, Spacing::SM, LV_PART_MAIN);
    lv_obj_set_style_pad_top(item, Spacing::XS, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(item, Spacing::XS, LV_PART_MAIN);

    uint32_t childCount = lv_obj_get_child_cnt(item);
    for (uint32_t i = 0; i < childCount; ++i) {
        lv_obj_t* child = lv_obj_get_child(item, i);
        if (!child) continue;
        if (lv_obj_check_type(child, &lv_label_class)) {
            lv_obj_set_style_text_color(child, rgb565ToLVGL(Colors::LIST_ITEM_TEXT), LV_PART_MAIN);
            lv_obj_set_style_text_font(child, Fonts::get(fontSize), LV_PART_MAIN);
            lv_obj_set_style_text_align(child, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
            lv_label_set_long_mode(child, LV_LABEL_LONG_WRAP);
        }
    }
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

