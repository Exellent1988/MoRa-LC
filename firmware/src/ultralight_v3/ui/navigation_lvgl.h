#ifndef NAVIGATION_LVGL_H
#define NAVIGATION_LVGL_H

#include <Arduino.h>
#include "screens/lvgl_base_screen.h"
#include <lvgl.h>

/**
 * Navigation System for LVGL8 Screens
 * Manages screen transitions and navigation stack
 */
class LVGLNavigation {
public:
    LVGLNavigation();
    ~LVGLNavigation();
    
    // Initialization
    void begin(LVGLDisplay* lvglDisplay);
    
    // Screen management
    void setScreen(LVGLBaseScreen* screen);
    void goBack();
    
    // Access
    LVGLBaseScreen* getCurrentScreen() { return _currentScreen; }
    bool canGoBack() const { return _previousScreen != nullptr; }
    
private:
    LVGLDisplay* _lvglDisplay;
    LVGLBaseScreen* _currentScreen;
    LVGLBaseScreen* _previousScreen;
    
    void transitionToScreen(LVGLBaseScreen* newScreen);
};

#endif // NAVIGATION_LVGL_H

