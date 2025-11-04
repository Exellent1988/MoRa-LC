#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include "screens/base_screen.h"

// Forward declarations
class Display;
class TouchHandler;

/**
 * Screen Navigation Manager
 * Handles screen transitions and routing
 */
class Navigation {
public:
    Navigation();
    ~Navigation();
    
    // Initialization
    void begin(Display* display, TouchHandler* touch);
    
    // Screen management
    void setScreen(BaseScreen* screen);
    BaseScreen* getCurrentScreen() const { return _currentScreen; }
    BaseScreen* getPreviousScreen() const { return _previousScreen; }
    
    // Navigation
    void goBack();
    bool canGoBack() const { return _previousScreen != nullptr; }
    
    // Update loop
    void update();
    
private:
    Display* _display;
    TouchHandler* _touch;
    BaseScreen* _currentScreen;
    BaseScreen* _previousScreen;
    
    void transitionToScreen(BaseScreen* newScreen);
};

#endif // NAVIGATION_H

