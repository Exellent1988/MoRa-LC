#ifndef BASE_SCREEN_H
#define BASE_SCREEN_H

#include <Arduino.h>
#include "../theme.h"

// Forward declarations
class Display;
class TouchHandler;

/**
 * Base Screen Class
 * All screens inherit from this
 */
class BaseScreen {
public:
    BaseScreen();
    virtual ~BaseScreen();
    
    // Core screen methods
    virtual void draw() = 0;
    virtual void update(uint32_t deltaTime) {}
    virtual bool handleTouch(uint16_t x, uint16_t y) = 0;
    
    // Lifecycle hooks
    virtual void onEnter() {}
    virtual void onExit() {}
    
    // State
    bool needsRedraw() const { return _needsRedraw; }
    void setNeedsRedraw(bool needs = true) { _needsRedraw = needs; }
    
    // Helper methods for common UI elements
    void drawHeader(const String& title, bool showBack = false);
    void drawFooter();
    
    // Access to hardware (will be set by Navigation)
    Display* _display;
    TouchHandler* _touch;

protected:
    
private:
    bool _needsRedraw;
};

#endif // BASE_SCREEN_H

