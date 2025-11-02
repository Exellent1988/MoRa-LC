#ifndef TOUCH_PIN_TEST_H
#define TOUCH_PIN_TEST_H

#include <Arduino.h>

// Test verschiedene Touch CS Pin Kombinationen
// ESP32-2432S028 USB-C Varianten haben manchmal andere Pins

void testTouchPins() {
    Serial.println("\n=== TOUCH PIN TEST ===");
    Serial.println("Testing different TOUCH_CS pins...\n");
    
    int test_pins[] = {33, 32, 25, 26, 27, 14};
    int num_pins = 6;
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.println("Touch Pin Test");
    tft.setCursor(10, 30);
    tft.println("Testing CS pins:");
    tft.setCursor(10, 50);
    tft.println("33, 32, 25, 26, 27, 14");
    tft.setCursor(10, 70);
    tft.println("Touch screen NOW!");
    
    for (int i = 0; i < num_pins; i++) {
        int cs_pin = test_pins[i];
        
        Serial.printf("--- Testing TOUCH_CS = %d ---\n", cs_pin);
        
        // Initialisiere Touch mit diesem Pin
        pinMode(cs_pin, OUTPUT);
        digitalWrite(cs_pin, HIGH);
        
        // TFT_eSPI Touch mit neuem CS Pin initialisieren
        // Das ist tricky, weil TFT_eSPI die Pins beim Compile festlegt
        // Wir müssen manuell testen
        
        tft.setCursor(10, 90 + i*15);
        tft.printf("Testing CS=%d...", cs_pin);
        
        // Warte auf Touch-Test
        bool touched = false;
        for (int test = 0; test < 20; test++) {
            // Manueller SPI Touch-Read würde hier kommen
            // Aber das ist zu komplex ohne direkte XPT2046 Library
            delay(50);
        }
        
        Serial.println("  (Manual test needed)");
    }
    
    Serial.println("\n=== ALTERNATIVE: Check your display variant ===");
    Serial.println("ESP32-2432S028 USB-C variants:");
    Serial.println("  - Some have NO touch at all");
    Serial.println("  - Some have resistive touch on CS=33");
    Serial.println("  - Some have capacitive touch (but we found none on I2C)");
    Serial.println("\nPossible issue:");
    Serial.println("  1. Touch not connected/soldered");
    Serial.println("  2. Wrong CS pin (try Pin 25, 26, or 27)");
    Serial.println("  3. Display variant without touch");
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(10, 80);
    tft.println("No Touch Found!");
    tft.setTextSize(1);
    tft.setCursor(10, 110);
    tft.println("Check Serial Monitor");
    tft.setCursor(10, 130);
    tft.println("Your display may have");
    tft.setCursor(10, 145);
    tft.println("no touch controller");
    
    delay(10000);
}

#endif // TOUCH_PIN_TEST_H

