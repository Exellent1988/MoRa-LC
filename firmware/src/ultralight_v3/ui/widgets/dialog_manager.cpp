#include "dialog_manager.h"
#include <Arduino.h>

// Initialize static instance pointer
DialogManager* DialogManager::_instance = nullptr;

DialogManager::DialogManager()
    : _dialog(nullptr)
    , _rootScreen(nullptr) {
}

DialogManager::~DialogManager() {
    if (_dialog) {
        delete _dialog;
        _dialog = nullptr;
    }
}

DialogManager* DialogManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new DialogManager();
        Serial.println("[DialogManager] Singleton instance created");
    }
    return _instance;
}

void DialogManager::begin(lv_obj_t* rootScreen) {
    if (!rootScreen) {
        Serial.println("[DialogManager] ERROR: rootScreen is null");
        return;
    }
    
    _rootScreen = rootScreen;
    
    // Create dialog instance
    if (!_dialog) {
        _dialog = new LVGLDialog(_rootScreen);
        Serial.println("[DialogManager] Dialog instance created");
    }
}

void DialogManager::show(const char* title, const char* message, 
                         const char* buttonText,
                         lv_event_cb_t callback, void* user_data) {
    if (!_dialog) {
        Serial.println("[DialogManager] ERROR: Dialog not initialized. Call begin() first!");
        return;
    }
    
    _dialog->show(title, message, buttonText, callback, user_data);
}

void DialogManager::showConfirm(const char* title, const char* message,
                                 const char* okText, const char* cancelText,
                                 lv_event_cb_t okCallback, lv_event_cb_t cancelCallback,
                                 void* user_data) {
    if (!_dialog) {
        Serial.println("[DialogManager] ERROR: Dialog not initialized. Call begin() first!");
        return;
    }
    
    _dialog->showConfirm(title, message, okText, cancelText, okCallback, cancelCallback, user_data);
}

void DialogManager::hide() {
    if (_dialog) {
        _dialog->hide();
    }
}

bool DialogManager::isVisible() const {
    if (_dialog) {
        return _dialog->isVisible();
    }
    return false;
}
