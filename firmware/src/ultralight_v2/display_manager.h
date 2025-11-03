#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "LGFX_Config.hpp"  // Must be included before using LGFX
#include "config.h"

// LovyanGFX Display Manager for ESP32-2432S028
// Uses ST7789 driver with XPT2046 touch

class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();
    
    bool begin();
    bool isReady() const { return _initialized; }
    
    // Display functions
    LGFX& getDisplay() { return _display; }
    
    // Touch functions
    bool getTouch(uint16_t* x, uint16_t* y);
    bool isTouched();
    bool calibrateTouch();
    
    // Utility
    void fillScreen(uint16_t color);
    void setTextColor(uint16_t color);
    void setTextSize(uint8_t size);
    void setCursor(int16_t x, int16_t y);
    void print(const String& text);
    void println(const String& text);
    void fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color);
    
private:
    LGFX _display;
    bool _initialized;
};

#endif // DISPLAY_MANAGER_H

