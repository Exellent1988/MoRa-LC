#ifndef TOUCH_GT911_H
#define TOUCH_GT911_H

#include <Arduino.h>
#include <Wire.h>
#include <TAMC_GT911.h>

// GT911 Touch Controller Wrapper
// für ESP32-2432S028Rv2/Rv3 mit USB-C

class TouchGT911 {
public:
    TouchGT911() : gt911(nullptr), initialized(false) {}
    
    bool begin() {
        Serial.println("[Touch GT911] Initializing...");
        
        // I2C für Touch initialisieren
        Wire.begin(TOUCH_SDA, TOUCH_SCL);
        
        // GT911 initialisieren
        gt911 = new TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, 
                               max(TFT_WIDTH, TFT_HEIGHT), 
                               min(TFT_WIDTH, TFT_HEIGHT));
        
        gt911->begin();
        gt911->setRotation(ROTATION_NORMAL);  // Landscape
        
        initialized = true;
        Serial.println("[Touch GT911] Initialized");
        return true;
    }
    
    bool getTouch(uint16_t* x, uint16_t* y) {
        if (!initialized || !gt911) return false;
        
        gt911->read();
        
        if (gt911->isTouched) {
            // Koordinaten für Landscape Modus
            *x = gt911->points[0].x;
            *y = gt911->points[0].y;
            
            return true;
        }
        
        return false;
    }
    
    bool isPressed() {
        if (!initialized || !gt911) return false;
        
        gt911->read();
        return gt911->isTouched;
    }
    
private:
    TAMC_GT911* gt911;
    bool initialized;
};

#endif // TOUCH_GT911_H

