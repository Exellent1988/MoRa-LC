#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "../core/config.h"
#include "LGFX_Config.hpp"

/**
 * Display Manager - LovyanGFX Wrapper
 * Minimal wrapper around LovyanGFX for clean interface
 */
class Display {
public:
    Display();
    ~Display();
    
    // Initialization
    bool begin();
    bool isReady() const { return _initialized; }
    
    // Access to LovyanGFX instance
    LGFX& getLGFX() { return _lgfx; }
    
    // Convenience methods
    void fillScreen(uint16_t color);
    void setTextColor(uint16_t color);
    void setTextSize(uint8_t size);
    void setCursor(int16_t x, int16_t y);
    void print(const String& text);
    void println(const String& text);
    
    // Touch
    bool getTouch(uint16_t* x, uint16_t* y);
    bool isTouched();
    
private:
    LGFX _lgfx;
    bool _initialized;
    
    void configureLGFX();
};

#endif // DISPLAY_H

