#include "navigation_lvgl.h"
#include <Arduino.h>

LVGLNavigation::LVGLNavigation()
    : _lvglDisplay(nullptr)
    , _currentScreen(nullptr)
    , _previousScreen(nullptr)
    , _navigationStack() {
}

LVGLNavigation::~LVGLNavigation() {
}

void LVGLNavigation::begin(LVGLDisplay* lvglDisplay) {
    _lvglDisplay = lvglDisplay;
}

void LVGLNavigation::setScreen(LVGLBaseScreen* screen) {
    transitionToScreen(screen, false);
}

void LVGLNavigation::transitionToScreen(LVGLBaseScreen* newScreen, bool isBackNavigation) {
    if (!newScreen) {
        Serial.println("[LVGLNavigation] ERROR: newScreen is null!");
        return;
    }
    
    if (newScreen == _currentScreen && !isBackNavigation) {
        Serial.println("[LVGLNavigation] Screen is already current, skipping transition");
        return;
    }
    
    Serial.printf("[LVGLNavigation] Transitioning to new screen: %p (back=%d)\n", newScreen, isBackNavigation);
    Serial.printf("[LVGLNavigation] Current screen: %p, Previous: %p\n", _currentScreen, _previousScreen);
    
    // Exit current screen
    if (_currentScreen) {
        Serial.println("[LVGLNavigation] Exiting current screen");
        _currentScreen->onExit();
    }
    
    // Update screen references using navigation stack
    if (!isBackNavigation) {
        // Forward navigation: push current screen to stack and set as previous
        if (_currentScreen) {
            // Check if screen already exists in stack (prevent duplicates)
            bool alreadyInStack = false;
            for (const auto& screen : _navigationStack) {
                if (screen == _currentScreen) {
                    alreadyInStack = true;
                    Serial.println("[LVGLNavigation] WARNING: Screen already in stack, not adding duplicate");
                    break;
                }
            }
            
            // Only add if not a duplicate and stack not at limit
            if (!alreadyInStack) {
                if (_navigationStack.size() >= MAX_NAVIGATION_STACK_DEPTH) {
                    Serial.printf("[LVGLNavigation] WARNING: Navigation stack at limit (%d), removing oldest entry\n", MAX_NAVIGATION_STACK_DEPTH);
                    _navigationStack.erase(_navigationStack.begin());
                }
                _navigationStack.push_back(_currentScreen);
                Serial.printf("[LVGLNavigation] Stack size: %zu\n", _navigationStack.size());
            }
        }
        _previousScreen = _currentScreen;
    } else {
        // Back navigation: pop from stack to get the screen before the one we're going to
        if (!_navigationStack.empty()) {
            _navigationStack.pop_back();  // Remove the screen we're going back from
            if (!_navigationStack.empty()) {
                _previousScreen = _navigationStack.back();  // Get the screen before the target
            } else {
                _previousScreen = nullptr;  // No more screens in stack
            }
        } else {
            _previousScreen = nullptr;
        }
    }
    _currentScreen = newScreen;
    
    // Enter new screen
    Serial.println("[LVGLNavigation] Entering new screen");
    _currentScreen->onEnter();
    Serial.printf("[LVGLNavigation] Transition completed. Current: %p, Previous: %p\n", _currentScreen, _previousScreen);
}

void LVGLNavigation::goBack() {
    if (_previousScreen) {
        Serial.printf("[LVGLNavigation] Going back to previous screen: %p (stack size: %zu)\n", _previousScreen, _navigationStack.size());
        LVGLBaseScreen* targetScreen = _previousScreen;
        // Save the screen we're currently on (for potential future use)
        LVGLBaseScreen* currentBeforeBack = _currentScreen;
        
        // Navigate back
        transitionToScreen(targetScreen, true);
        
        // After going back, we want _previousScreen to point to the screen we came from
        // But actually, _previousScreen should point to the screen before the targetScreen
        // Since we don't have a full stack, we'll just keep _previousScreen as it was
        // which is correct for a single level of back navigation
        // The next forward navigation will update it correctly
    } else {
        Serial.println("[LVGLNavigation] WARNING: No previous screen to go back to!");
    }
}

