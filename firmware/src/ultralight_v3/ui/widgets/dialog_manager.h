#ifndef DIALOG_MANAGER_H
#define DIALOG_MANAGER_H

#include "lvgl_dialog.h"
#include <lvgl.h>

/**
 * Dialog Manager Singleton
 * Provides centralized dialog management for all screens
 * Prevents memory overhead from multiple dialog instances
 */
class DialogManager {
public:
    // Get singleton instance
    static DialogManager* getInstance();
    
    // Initialize with root screen
    void begin(lv_obj_t* rootScreen);
    
    // Show dialog methods (delegates to internal LVGLDialog)
    void show(const char* title, const char* message, 
              const char* buttonText = "OK",
              lv_event_cb_t callback = nullptr, void* user_data = nullptr);
    
    void showConfirm(const char* title, const char* message,
                     const char* okText = "OK", const char* cancelText = "Cancel",
                     lv_event_cb_t okCallback = nullptr, lv_event_cb_t cancelCallback = nullptr,
                     void* user_data = nullptr);
    
    // Hide current dialog
    void hide();
    
    // Check if dialog is visible
    bool isVisible() const;
    
private:
    // Private constructor for singleton
    DialogManager();
    ~DialogManager();
    
    // Delete copy constructor and assignment operator
    DialogManager(const DialogManager&) = delete;
    DialogManager& operator=(const DialogManager&) = delete;
    
    static DialogManager* _instance;
    LVGLDialog* _dialog;
    lv_obj_t* _rootScreen;
};

#endif // DIALOG_MANAGER_H
