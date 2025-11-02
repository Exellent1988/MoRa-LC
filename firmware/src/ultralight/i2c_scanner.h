#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <Wire.h>

// I2C Scanner to find touch controller
// Common touch controllers on ESP32-2432S028 USB-C variants:
// - GT911: 0x5D or 0x14
// - FT6236/FT6336: 0x38
// - CST816S: 0x15

void scanI2C() {
    Serial.println("\n=== I2C SCANNER ===");
    Serial.println("Scanning I2C bus...\n");
    
    // Try different I2C pin combinations
    int sda_pins[] = {21, 33, 32};
    int scl_pins[] = {22, 32, 33};
    int combinations = 3;
    
    for (int combo = 0; combo < combinations; combo++) {
        int sda = sda_pins[combo];
        int scl = scl_pins[combo];
        
        Serial.printf("--- Testing SDA=%d, SCL=%d ---\n", sda, scl);
        
        Wire.begin(sda, scl);
        delay(100);
        
        byte error, address;
        int nDevices = 0;
        
        for(address = 1; address < 127; address++) {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.printf("✓ I2C device found at 0x%02X", address);
                
                // Identify known touch controllers
                if (address == 0x5D || address == 0x14) {
                    Serial.print("  → GT911 Touch Controller!");
                } else if (address == 0x38) {
                    Serial.print("  → FT6236/FT6336 Touch Controller!");
                } else if (address == 0x15) {
                    Serial.print("  → CST816S Touch Controller!");
                }
                Serial.println();
                nDevices++;
            }
        }
        
        if (nDevices == 0) {
            Serial.println("  No I2C devices found");
        }
        
        Wire.end();
        delay(100);
    }
    
    Serial.println("\n=== I2C SCAN COMPLETE ===");
    Serial.println("Check above for detected touch controller.");
    Serial.println("Note the address and SDA/SCL pins!\n");
}

#endif // I2C_SCANNER_H

