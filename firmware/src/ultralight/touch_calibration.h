// Touch Calibration Helper for CheapYellow Display
// 
// If touch doesn't work or is inverted/offset:
// 1. Uncomment TOUCH_CALIBRATION_MODE below
// 2. Upload firmware
// 3. Follow on-screen instructions
// 4. Note the calibration values from Serial Monitor
// 5. Update calData[] in main.cpp
// 6. Comment out TOUCH_CALIBRATION_MODE again

// #define TOUCH_CALIBRATION_MODE  // Disabled - using XPT2046
// #define RUN_I2C_SCANNER  // Disabled - no I2C touch found
// #define TEST_TOUCH_PINS  // Disabled - testing Pin 21

#ifdef TOUCH_CALIBRATION_MODE

#ifdef RUN_I2C_SCANNER
#include "i2c_scanner.h"
#endif

#ifdef TEST_TOUCH_PINS
#include "touch_pin_test.h"
#endif

void runTouchCalibration() {
    #ifdef RUN_I2C_SCANNER
    // Run I2C scanner first to find touch controller
    scanI2C();
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 100);
    tft.println("I2C Scan Done!");
    tft.setTextSize(1);
    tft.setCursor(10, 130);
    tft.println("Check Serial Monitor");
    tft.setCursor(10, 150);
    tft.println("for touch controller");
    
    delay(5000);
    #endif
    
    #ifdef TEST_TOUCH_PINS
    testTouchPins();
    #endif
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 20);
    tft.println("TOUCH TEST MODE");
    tft.setTextSize(1);
    tft.setCursor(20, 60);
    tft.println("Touch anywhere...");
    tft.setCursor(20, 80);
    tft.println("Watch Serial Monitor");
    
    Serial.println("\n\n=== TOUCH TEST MODE ===");
    Serial.println("Touch the screen anywhere...");
    Serial.println("Raw touch data will be printed\n");
    
    // Test if touch responds at all
    for (int i = 0; i < 100; i++) {
        uint16_t x, y;
        if (tft.getTouch(&x, &y)) {
            Serial.printf("[TOUCH DETECTED] Raw X=%u, Y=%u\n", x, y);
            
            tft.fillCircle(x, y, 3, TFT_GREEN);
            
            delay(100);
        }
        delay(50);
    }
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 20);
    tft.println("Starting calibration");
    tft.setTextSize(1);
    tft.setCursor(20, 60);
    tft.println("Touch corners when");
    tft.setCursor(20, 80);
    tft.println("crosshair appears");
    
    delay(2000);
    
    uint16_t calData[5];
    
    // Touch calibration
    tft.fillScreen(TFT_BLACK);
    tft.calibrateTouch(calData, TFT_WHITE, TFT_RED, 15);
    
    Serial.println("\n\n=== TOUCH CALIBRATION DATA ===");
    Serial.print("uint16_t calData[5] = { ");
    Serial.print(calData[0]);
    Serial.print(", ");
    Serial.print(calData[1]);
    Serial.print(", ");
    Serial.print(calData[2]);
    Serial.print(", ");
    Serial.print(calData[3]);
    Serial.print(", ");
    Serial.print(calData[4]);
    Serial.println(" };");
    Serial.println("=================================\n");
    Serial.println("Copy these values to main.cpp line 154");
    Serial.println("Then comment out TOUCH_CALIBRATION_MODE\n");
    
    tft.setTouch(calData);
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 100);
    tft.println("Calibration done!");
    tft.setCursor(20, 120);
    tft.println("Check Serial Monitor");
    
    delay(5000);
}

#endif // TOUCH_CALIBRATION_MODE

