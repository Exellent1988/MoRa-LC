#include "lvgl_dialog.h"
#include <Arduino.h>

LVGLDialog::LVGLDialog(lv_obj_t* parent)
    : _parent(parent)
    , _obj(nullptr)
    , _titleLabel(nullptr)
    , _messageLabel(nullptr)
    , _okButton(nullptr)
    , _cancelButton(nullptr) {
}

LVGLDialog::~LVGLDialog() {
    hide();
}

void LVGLDialog::show(const char* title, const char* message, 
                     const char* buttonText,
                     lv_event_cb_t callback, void* user_data) {
    hide(); // Remove existing dialog
    
    // Create modal background
    _obj = lv_obj_create(_parent);
    lv_obj_set_size(_obj, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(_obj, 0, 0);
    lv_obj_set_style_bg_color(_obj, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(_obj, LV_OPA_50, 0);
    lv_obj_set_style_border_width(_obj, 0, 0);
    
    // Create dialog box
    lv_obj_t* dialogBox = lv_obj_create(_obj);
    lv_obj_set_size(dialogBox, SCREEN_WIDTH - 80, 150);
    lv_obj_align(dialogBox, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(dialogBox, rgb565ToLVGL(Colors::SURFACE), 0);
    lv_obj_set_style_border_width(dialogBox, 2, 0);
    lv_obj_set_style_border_color(dialogBox, rgb565ToLVGL(Colors::BORDER), 0);
    
    // Title
    _titleLabel = lv_label_create(dialogBox);
    lv_label_set_text(_titleLabel, title);
    lv_obj_set_style_text_color(_titleLabel, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_style_text_font(_titleLabel, Fonts::get(Fonts::Size::Subtitle), 0);
    lv_obj_align(_titleLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // Message
    _messageLabel = lv_label_create(dialogBox);
    lv_label_set_text(_messageLabel, message);
    lv_obj_set_style_text_color(_messageLabel, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_style_text_font(_messageLabel, Fonts::get(Fonts::Size::Body), 0);
    lv_obj_align(_messageLabel, LV_ALIGN_CENTER, 0, -10);
    lv_label_set_long_mode(_messageLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_messageLabel, SCREEN_WIDTH - 120);
    
    // OK button
    _okButton = lv_btn_create(dialogBox);
    lv_obj_set_size(_okButton, 100, 35);
    lv_obj_align(_okButton, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(_okButton, rgb565ToLVGL(Colors::SECONDARY), 0);
    
    lv_obj_t* okLabel = lv_label_create(_okButton);
    lv_label_set_text(okLabel, buttonText);
    lv_obj_set_style_text_color(okLabel, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    lv_obj_set_style_text_font(okLabel, Fonts::get(Fonts::Size::Body), 0);
    lv_obj_center(okLabel);
    
    if (callback) {
        lv_obj_add_event_cb(_okButton, callback, LV_EVENT_CLICKED, user_data);
    }
    lv_obj_add_event_cb(_okButton, [](lv_event_t* e) {
        lv_obj_t* dialog = lv_obj_get_parent(lv_obj_get_parent(lv_event_get_target(e)));
        lv_obj_del(dialog);
    }, LV_EVENT_CLICKED, nullptr);
}

void LVGLDialog::showConfirm(const char* title, const char* message,
                             const char* okText, const char* cancelText,
                             lv_event_cb_t okCallback, lv_event_cb_t cancelCallback,
                             void* user_data) {
    hide();
    
    // Create modal background
    _obj = lv_obj_create(_parent);
    lv_obj_set_size(_obj, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(_obj, 0, 0);
    lv_obj_set_style_bg_color(_obj, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(_obj, LV_OPA_50, 0);
    
    // Create dialog box
    lv_obj_t* dialogBox = lv_obj_create(_obj);
    lv_obj_set_size(dialogBox, SCREEN_WIDTH - 80, 150);
    lv_obj_align(dialogBox, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(dialogBox, rgb565ToLVGL(Colors::SURFACE), 0);
    
    // Title
    _titleLabel = lv_label_create(dialogBox);
    lv_label_set_text(_titleLabel, title);
    lv_obj_align(_titleLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // Message
    _messageLabel = lv_label_create(dialogBox);
    lv_label_set_text(_messageLabel, message);
    lv_obj_align(_messageLabel, LV_ALIGN_CENTER, 0, -10);
    lv_label_set_long_mode(_messageLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_messageLabel, SCREEN_WIDTH - 120);
    
    // Buttons
    int btnW = 80;
    int btnH = 35;
    int spacing = 20;
    
    _okButton = lv_btn_create(dialogBox);
    lv_obj_set_size(_okButton, btnW, btnH);
    lv_obj_align(_okButton, LV_ALIGN_BOTTOM_LEFT, spacing, -10);
    lv_obj_set_style_bg_color(_okButton, rgb565ToLVGL(Colors::SECONDARY), 0);
    
    lv_obj_t* okLabel = lv_label_create(_okButton);
    lv_label_set_text(okLabel, okText);
    lv_obj_set_style_text_font(okLabel, Fonts::get(Fonts::Size::Body), 0);
    lv_obj_center(okLabel);
    
    if (okCallback) {
        lv_obj_add_event_cb(_okButton, okCallback, LV_EVENT_CLICKED, user_data);
    }
    lv_obj_add_event_cb(_okButton, okButtonHandler, LV_EVENT_CLICKED, this);
    
    _cancelButton = lv_btn_create(dialogBox);
    lv_obj_set_size(_cancelButton, btnW, btnH);
    lv_obj_align(_cancelButton, LV_ALIGN_BOTTOM_RIGHT, -spacing, -10);
    lv_obj_set_style_bg_color(_cancelButton, rgb565ToLVGL(Colors::BUTTON), 0);
    
    lv_obj_t* cancelLabel = lv_label_create(_cancelButton);
    lv_label_set_text(cancelLabel, cancelText);
    lv_obj_set_style_text_font(cancelLabel, Fonts::get(Fonts::Size::Body), 0);
    lv_obj_center(cancelLabel);
    
    if (cancelCallback) {
        lv_obj_add_event_cb(_cancelButton, cancelCallback, LV_EVENT_CLICKED, user_data);
    }
    lv_obj_add_event_cb(_cancelButton, cancelButtonHandler, LV_EVENT_CLICKED, this);
}

void LVGLDialog::hide() {
    if (_obj) {
        lv_obj_del(_obj);
        _obj = nullptr;
        _titleLabel = nullptr;
        _messageLabel = nullptr;
        _okButton = nullptr;
        _cancelButton = nullptr;
    }
}

bool LVGLDialog::isVisible() const {
    return _obj != nullptr;
}

void LVGLDialog::okButtonHandler(lv_event_t* e) {
    LVGLDialog* dialog = (LVGLDialog*)lv_event_get_user_data(e);
    if (dialog) {
        dialog->hide();
    }
}

void LVGLDialog::cancelButtonHandler(lv_event_t* e) {
    LVGLDialog* dialog = (LVGLDialog*)lv_event_get_user_data(e);
    if (dialog) {
        dialog->hide();
    }
}

lv_color_t LVGLDialog::rgb565ToLVGL(uint16_t rgb565) {
    lv_color_t color;
    color.full = rgb565;
    return color;
}

