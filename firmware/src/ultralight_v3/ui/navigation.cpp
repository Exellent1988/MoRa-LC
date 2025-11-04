#include "navigation.h"
#include "../hardware/display.h"
#include "../hardware/touch.h"
#include "screens/base_screen.h"

Navigation::Navigation() 
    : _display(nullptr)
    , _touch(nullptr)
    , _currentScreen(nullptr)
    , _previousScreen(nullptr) {
}

Navigation::~Navigation() {
}

void Navigation::begin(Display* display, TouchHandler* touch) {
    _display = display;
    _touch = touch;
}

void Navigation::setScreen(BaseScreen* screen) {
    transitionToScreen(screen);
}

void Navigation::transitionToScreen(BaseScreen* newScreen) {
    if (newScreen == _currentScreen) {
        return;
    }
    
    Serial.printf("[Navigation] Transitioning to new screen: %p\n", newScreen);
    
    // Exit current screen
    if (_currentScreen) {
        Serial.println("[Navigation] Exiting current screen");
        _currentScreen->onExit();
    }
    
    // Update screen references
    _previousScreen = _currentScreen;
    _currentScreen = newScreen;
    
    // Set hardware references
    if (_currentScreen) {
        Serial.printf("[Navigation] Setting display=%p, touch=%p\n", _display, _touch);
        _currentScreen->_display = _display;
        _currentScreen->_touch = _touch;
        Serial.println("[Navigation] Calling onEnter()...");
        _currentScreen->onEnter();
        Serial.println("[Navigation] Setting needsRedraw=true");
        _currentScreen->setNeedsRedraw(true);
    } else {
        Serial.println("[Navigation] ERROR: newScreen is null!");
    }
}

void Navigation::goBack() {
    if (_previousScreen) {
        transitionToScreen(_previousScreen);
    }
}

void Navigation::update() {
    if (!_currentScreen) {
        // Serial.println("[Navigation] No current screen");
        return;
    }
    
    // Update screen
    uint32_t deltaTime = 1; // TODO: Calculate actual delta
    _currentScreen->update(deltaTime);
    
    // Redraw if needed
    if (_currentScreen->needsRedraw()) {
        // Serial.println("[Navigation] Redraw needed, calling draw()");
        _currentScreen->draw();
        _currentScreen->setNeedsRedraw(false);
    }
    
    // Handle touch
    uint16_t x, y;
    if (_touch && _touch->getTouch(&x, &y)) {
        // Debug: Print touch coordinates (uncomment for testing)
        // Serial.printf("[Navigation] Touch at x=%u, y=%u\n", x, y);
        _currentScreen->handleTouch(x, y);
    }
}

