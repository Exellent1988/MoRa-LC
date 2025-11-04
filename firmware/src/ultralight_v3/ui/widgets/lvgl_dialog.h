#ifndef LVGL_DIALOG_H
#define LVGL_DIALOG_H

#include <lvgl.h>
#include "../theme.h"

/**
 * LVGL Dialog Widget
 * Modal dialogs for confirmations and messages
 */
class LVGLDialog {
public:
    LVGLDialog(lv_obj_t* parent);
    ~LVGLDialog();
    
    // Show dialog
    void show(const char* title, const char* message, 
              const char* buttonText = "OK",
              lv_event_cb_t callback = nullptr, void* user_data = nullptr);
    
    void showConfirm(const char* title, const char* message,
                     const char* okText = "OK", const char* cancelText = "Abbrechen",
                     lv_event_cb_t okCallback = nullptr, lv_event_cb_t cancelCallback = nullptr,
                     void* user_data = nullptr);
    
    // Hide dialog
    void hide();
    
    // State
    bool isVisible() const;
    
private:
    lv_obj_t* _parent;
    lv_obj_t* _obj;
    lv_obj_t* _titleLabel;
    lv_obj_t* _messageLabel;
    lv_obj_t* _okButton;
    lv_obj_t* _cancelButton;
    
    static void okButtonHandler(lv_event_t* e);
    static void cancelButtonHandler(lv_event_t* e);
    
    static lv_color_t rgb565ToLVGL(uint16_t rgb565);
};

#endif // LVGL_DIALOG_H

