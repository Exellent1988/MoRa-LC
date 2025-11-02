#ifndef TOUCH_XPT2046_H
#define TOUCH_XPT2046_H

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "config.h"

// XPT2046 Touch Controller Wrapper
// für ESP32-2432S028 (CYD - Cheap Yellow Display)

class TouchXPT2046 {
public:
    TouchXPT2046() : ts(nullptr), initialized(false) {}
    
    bool begin() {
        Serial.println("[Touch XPT2046] Initializing...");
        Serial.printf("[Touch XPT2046] Pins: CS=%d, IRQ=%d, MOSI=%d, MISO=%d, CLK=%d\n", 
                      XPT2046_CS, XPT2046_IRQ, XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK);
        
        // Explizit SPI initialisieren mit korrekten Pins
        SPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
        
        // XPT2046 initialisieren (nutzt automatisch das SPI Object)
        ts = new XPT2046_Touchscreen(XPT2046_CS, XPT2046_IRQ);
        ts->begin();
        ts->setRotation(1);  // Landscape
        
        initialized = true;
        Serial.println("[Touch XPT2046] Initialized");
        return true;
    }
    
    bool getTouch(uint16_t* x, uint16_t* y) {
        if (!initialized || !ts) return false;
        
        if (ts->touched()) {
            TS_Point p = ts->getPoint();
            
            // Debug: Raw values anzeigen
            static uint32_t lastDebug = 0;
            if (millis() - lastDebug > 1000) {
                Serial.printf("[Touch XPT2046] Raw: x=%d, y=%d, z=%d\n", p.x, p.y, p.z);
                lastDebug = millis();
            }
            
            // Map touch coordinates to screen coordinates
            // XPT2046 raw: 0-4095 (typisch 200-3800)
            // Screen: 0-320 (width) x 0-240 (height) in landscape
            *x = map(p.x, 200, 3700, 0, 320);
            *y = map(p.y, 200, 3800, 0, 240);
            
            // Clamp to screen bounds
            *x = constrain(*x, 0, 319);
            *y = constrain(*y, 0, 239);
            
            return true;
        }
        
        return false;
    }
    
    bool isPressed() {
        if (!initialized || !ts) return false;
        return ts->touched();
    }
    
private:
    XPT2046_Touchscreen* ts;
    bool initialized;
};

#endif // TOUCH_XPT2046_H

