#ifndef UI_KEYBOARD_H
#define UI_KEYBOARD_H

#include <Arduino.h>
#include "config.h"
#include "display_manager.h"

/**
 * On-Screen Keyboard für Text-Eingabe (LovyanGFX Version)
 */

// Keyboard Layout
const char KEYBOARD_LAYOUT[4][10] = {
    {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
    {'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P'},
    {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '-'},
    {'Y', 'X', 'C', 'V', 'B', 'N', 'M', ' ', '<', '>'}
};

class OnScreenKeyboard {
public:
    OnScreenKeyboard(DisplayManager& disp);
    
    // Show keyboard and get text input
    String getText(const String& prompt, const String& defaultText = "", uint8_t maxLength = 20);
    
    // Show number picker
    uint32_t getNumber(const String& prompt, uint32_t defaultValue, uint32_t minVal, uint32_t maxVal);
    
private:
    DisplayManager& display;
    String currentText;
    String promptText;
    uint8_t maxLen;
    bool done;
    bool cancelled;
    
    // Keyboard dimensions (optimiert für 320x240 Display)
    static const int KEY_WIDTH = 28;
    static const int KEY_HEIGHT = 24;
    static const int KEY_SPACING = 2;
    static const int KEYBOARD_START_Y = 82;
    
    // Number picker
    int32_t currentNumber;
    int32_t minValue;
    int32_t maxValue;
    
    // Drawing
    void drawKeyboard();
    void drawKey(int row, int col, bool pressed = false);
    void drawTextArea();
    void drawNumberPicker();
    
    // Touch handling
    bool getTouchKey(uint16_t x, uint16_t y, int& row, int& col);
    bool isTouchInKey(uint16_t x, uint16_t y, int row, int col);
    bool isTouchInRect(uint16_t tx, uint16_t ty, int x, int y, int w, int h);
    
    // Special keys
    void handleBackspace();
    void handleDone();
    void handleCancel();
};

#endif // UI_KEYBOARD_H

