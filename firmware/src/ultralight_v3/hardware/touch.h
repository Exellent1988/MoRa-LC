#ifndef TOUCH_H
#define TOUCH_H

#include <Arduino.h>
#include "../core/config.h"

/**
 * Touch Input Handler
 * Handles touch input with debouncing and calibration
 */
class TouchHandler {
public:
    TouchHandler();
    ~TouchHandler();
    
    // Initialization
    void begin(class Display* display);
    
    // Touch state
    bool isTouched();
    bool getTouch(uint16_t* x, uint16_t* y);
    
    // Debouncing
    void setDebounce(uint32_t ms) { _debounceMs = ms; }
    uint32_t getDebounce() const { return _debounceMs; }
    
    // Last touch info
    uint16_t getLastX() const { return _lastX; }
    uint16_t getLastY() const { return _lastY; }
    uint32_t getLastTouchTime() const { return _lastTouchTime; }
    
private:
    Display* _display;
    uint32_t _debounceMs;
    uint16_t _lastX;
    uint16_t _lastY;
    uint32_t _lastTouchTime;
    bool _lastTouched;
};

#endif // TOUCH_H

