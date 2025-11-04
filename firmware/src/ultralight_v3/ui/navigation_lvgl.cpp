#include "navigation_lvgl.h"
#include <Arduino.h>

LVGLNavigation::LVGLNavigation()
    : _lvglDisplay(nullptr)
    , _currentScreen(nullptr)
    , _previousScreen(nullptr) {
}

LVGLNavigation::~LVGLNavigation() {
}

void LVGLNavigation::begin(LVGLDisplay* lvglDisplay) {
    _lvglDisplay = lvglDisplay;
}

void LVGLNavigation::setScreen(LVGLBaseScreen* screen) {
    transitionToScreen(screen);
}

void LVGLNavigation::transitionToScreen(LVGLBaseScreen* newScreen) {
    if (!newScreen) {
        Serial.println("[LVGLNavigation] ERROR: newScreen is null!");
        return;
    }
    
    if (newScreen == _currentScreen) {
        Serial.println("[LVGLNavigation] Screen is already current, skipping transition");
        return;
    }
    
    Serial.printf("[LVGLNavigation] Transitioning to new screen: %p\n", newScreen);
    Serial.printf("[LVGLNavigation] Current screen: %p, Previous: %p\n", _currentScreen, _previousScreen);
    
    // Exit current screen
    if (_currentScreen) {
        Serial.println("[LVGLNavigation] Exiting current screen");
        _currentScreen->onExit();
    }
    
    // Update screen references
    _previousScreen = _currentScreen;
    _currentScreen = newScreen;
    
    // Enter new screen
    Serial.println("[LVGLNavigation] Entering new screen");
    _currentScreen->onEnter();
    Serial.println("[LVGLNavigation] Transition completed");
}

void LVGLNavigation::goBack() {
    if (_previousScreen) {
        transitionToScreen(_previousScreen);
    }
}

