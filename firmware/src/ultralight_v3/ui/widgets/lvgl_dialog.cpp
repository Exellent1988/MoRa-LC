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
    hide();
    
    // Modal background on lv_layer_top
    _obj = lv_obj_create(lv_layer_top());
    lv_obj_set_size(_obj, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(_obj, 0, 0);
    lv_obj_set_style_bg_color(_obj, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(_obj, LV_OPA_50, 0);
    lv_obj_set_style_border_width(_obj, 0, 0);
    lv_obj_clear_flag(_obj, LV_OBJ_FLAG_SCROLLABLE);
    
    // Dialog box - use most of the screen (20px margin on each side)
    lv_obj_t* dialogBox = lv_obj_create(_obj);
    lv_obj_set_size(dialogBox, SCREEN_WIDTH - 40, SCREEN_HEIGHT - 40);
    lv_obj_align(dialogBox, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(dialogBox, rgb565ToLVGL(Colors::SURFACE), 0);
    lv_obj_set_style_border_width(dialogBox, 2, 0);
    lv_obj_set_style_border_color(dialogBox, rgb565ToLVGL(Colors::BORDER), 0);
    lv_obj_set_style_pad_all(dialogBox, 8, 0);
    
    // Title
    _titleLabel = lv_label_create(dialogBox);
    lv_label_set_text(_titleLabel, title);
    lv_obj_set_style_text_color(_titleLabel, rgb565ToLVGL(Colors::TEXT), 0);
    extern const lv_font_t lv_font_montserrat_16;
    lv_obj_set_style_text_font(_titleLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(_titleLabel, LV_ALIGN_TOP_MID, 0, 4);
    
    // Message - scrollable area between title and button
    _messageLabel = lv_label_create(dialogBox);
    lv_label_set_text(_messageLabel, message);
    lv_obj_set_style_text_color(_messageLabel, rgb565ToLVGL(Colors::TEXT), 0);
    lv_label_set_long_mode(_messageLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_messageLabel, SCREEN_WIDTH - 80);
    lv_obj_align(_messageLabel, LV_ALIGN_TOP_LEFT, 4, 28);
    
    // OK button at bottom
    _okButton = lv_btn_create(dialogBox);
    lv_obj_set_size(_okButton, 100, 32);
    lv_obj_align(_okButton, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_set_style_bg_color(_okButton, rgb565ToLVGL(Colors::SECONDARY), 0);
    
    lv_obj_t* okLabel = lv_label_create(_okButton);
    lv_label_set_text(okLabel, buttonText);
    lv_obj_set_style_text_color(okLabel, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    lv_obj_center(okLabel);
    
    if (callback) {
        lv_obj_add_event_cb(_okButton, callback, LV_EVENT_CLICKED, user_data);
    }
    lv_obj_add_event_cb(_okButton, okButtonHandler, LV_EVENT_CLICKED, this);
}

void LVGLDialog::showConfirm(const char* title, const char* message,
                             const char* okText, const char* cancelText,
                             lv_event_cb_t okCallback, lv_event_cb_t cancelCallback,
                             void* user_data) {
    hide();
    
    // Modal background on lv_layer_top
    _obj = lv_obj_create(lv_layer_top());
    lv_obj_set_size(_obj, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(_obj, 0, 0);
    lv_obj_set_style_bg_color(_obj, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(_obj, LV_OPA_50, 0);
    lv_obj_clear_flag(_obj, LV_OBJ_FLAG_SCROLLABLE);
    
    // Dialog box - use most of the screen
    lv_obj_t* dialogBox = lv_obj_create(_obj);
    lv_obj_set_size(dialogBox, SCREEN_WIDTH - 40, SCREEN_HEIGHT - 40);
    lv_obj_align(dialogBox, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(dialogBox, rgb565ToLVGL(Colors::SURFACE), 0);
    lv_obj_set_style_border_width(dialogBox, 2, 0);
    lv_obj_set_style_border_color(dialogBox, rgb565ToLVGL(Colors::BORDER), 0);
    lv_obj_set_style_pad_all(dialogBox, 8, 0);
    
    // Title
    _titleLabel = lv_label_create(dialogBox);
    lv_label_set_text(_titleLabel, title);
    lv_obj_set_style_text_color(_titleLabel, rgb565ToLVGL(Colors::TEXT), 0);
    extern const lv_font_t lv_font_montserrat_16;
    lv_obj_set_style_text_font(_titleLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(_titleLabel, LV_ALIGN_TOP_MID, 0, 4);
    
    // Message
    _messageLabel = lv_label_create(dialogBox);
    lv_label_set_text(_messageLabel, message);
    lv_obj_set_style_text_color(_messageLabel, rgb565ToLVGL(Colors::TEXT), 0);
    lv_label_set_long_mode(_messageLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_messageLabel, SCREEN_WIDTH - 80);
    lv_obj_align(_messageLabel, LV_ALIGN_TOP_LEFT, 4, 28);
    
    // Buttons at bottom
    int btnW = 90;
    int btnH = 32;
    
    _okButton = lv_btn_create(dialogBox);
    lv_obj_set_size(_okButton, btnW, btnH);
    lv_obj_align(_okButton, LV_ALIGN_BOTTOM_LEFT, 8, -4);
    lv_obj_set_style_bg_color(_okButton, rgb565ToLVGL(Colors::SECONDARY), 0);
    
    lv_obj_t* okLabel = lv_label_create(_okButton);
    lv_label_set_text(okLabel, okText);
    lv_obj_set_style_text_color(okLabel, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    lv_obj_center(okLabel);
    
    if (okCallback) {
        lv_obj_add_event_cb(_okButton, okCallback, LV_EVENT_CLICKED, user_data);
    }
    lv_obj_add_event_cb(_okButton, okButtonHandler, LV_EVENT_CLICKED, this);
    
    _cancelButton = lv_btn_create(dialogBox);
    lv_obj_set_size(_cancelButton, btnW, btnH);
    lv_obj_align(_cancelButton, LV_ALIGN_BOTTOM_RIGHT, -8, -4);
    lv_obj_set_style_bg_color(_cancelButton, rgb565ToLVGL(Colors::BUTTON), 0);
    
    lv_obj_t* cancelLabel = lv_label_create(_cancelButton);
    lv_label_set_text(cancelLabel, cancelText);
    lv_obj_set_style_text_color(cancelLabel, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
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

