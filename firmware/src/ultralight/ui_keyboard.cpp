#include "ui_keyboard.h"
#include "touch_wrapper.h"

extern TFT_eSPI tft;

OnScreenKeyboard::OnScreenKeyboard() 
    : currentText(""), promptText(""), maxLen(20), done(false), cancelled(false),
      currentNumber(0), minValue(0), maxValue(100) {
}

String OnScreenKeyboard::getText(const String& prompt, const String& defaultText, uint8_t maxLength) {
    promptText = prompt;
    currentText = defaultText;
    maxLen = maxLength;
    done = false;
    cancelled = false;
    
    drawKeyboard();
    drawTextArea();
    
    // Touch loop
    while (!done && !cancelled) {
        uint16_t x, y;
        if (getTouchCoordinates(&x, &y)) {
            // Debounce
            delay(50);
            while (getTouchCoordinates(&x, &y)) {
                delay(10);
            }
            
            // Check special buttons
            int btnY = 190;
            int btnH = 28;
            
            if (isTouchInRect(x, y, 10, btnY, 90, btnH)) {
                // Cancel
                cancelled = true;
                break;
            }
            
            if (isTouchInRect(x, y, 110, btnY, 90, btnH)) {
                // Done
                done = true;
                break;
            }
            
            // Check keyboard keys
            int row, col;
            if (getTouchKey(x, y, row, col)) {
                char key = KEYBOARD_LAYOUT[row][col];
                
                if (key == '<') {
                    // Backspace
                    handleBackspace();
                } else if (key == '>') {
                    // Done (alternative)
                    done = true;
                } else {
                    // Add character
                    if (currentText.length() < maxLen) {
                        currentText += key;
                        drawTextArea();
                    }
                }
            }
        }
        
        delay(10);
    }
    
    if (cancelled) {
        return defaultText;
    }
    
    return currentText;
}

uint32_t OnScreenKeyboard::getNumber(const String& prompt, uint32_t defaultValue, uint32_t minVal, uint32_t maxVal) {
    promptText = prompt;
    currentNumber = defaultValue;
    minValue = minVal;
    maxValue = maxVal;
    done = false;
    cancelled = false;
    
    drawNumberPicker();
    
    // Touch loop
    while (!done && !cancelled) {
        uint16_t x, y;
        if (getTouchCoordinates(&x, &y)) {
            // Debounce
            delay(50);
            while (getTouchCoordinates(&x, &y)) {
                delay(10);
            }
            
            int centerX = SCREEN_WIDTH / 2;
            int centerY = 120;
            
            // Plus button (+10)
            if (isTouchInRect(x, y, centerX + 60, centerY - 60, 50, 50)) {
                currentNumber = min((int32_t)maxValue, currentNumber + 10);
                drawNumberPicker();
            }
            // Plus button (+1)
            else if (isTouchInRect(x, y, centerX + 60, centerY, 50, 50)) {
                currentNumber = min((int32_t)maxValue, currentNumber + 1);
                drawNumberPicker();
            }
            // Minus button (-10)
            else if (isTouchInRect(x, y, centerX - 110, centerY - 60, 50, 50)) {
                currentNumber = max((int32_t)minValue, currentNumber - 10);
                drawNumberPicker();
            }
            // Minus button (-1)
            else if (isTouchInRect(x, y, centerX - 110, centerY, 50, 50)) {
                currentNumber = max((int32_t)minValue, currentNumber - 1);
                drawNumberPicker();
            }
            // Cancel
            else if (isTouchInRect(x, y, 10, SCREEN_HEIGHT - 50, 100, 40)) {
                cancelled = true;
            }
            // Done
            else if (isTouchInRect(x, y, 120, SCREEN_HEIGHT - 50, 100, 40)) {
                done = true;
            }
        }
        
        delay(10);
    }
    
    if (cancelled) {
        return defaultValue;
    }
    
    return currentNumber;
}

// ============================================================
// Drawing Functions
// ============================================================

void OnScreenKeyboard::drawKeyboard() {
    tft.fillScreen(BACKGROUND_COLOR);
    
    // Draw all keys
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 10; col++) {
            drawKey(row, col, false);
        }
    }
    
    // Draw control buttons (ganz unten, nach Keyboard)
    // Keyboard endet bei: 82 + 4*(24+2) = 82 + 104 = 186
    int btnY = 190;  // 4px Abstand nach Keyboard
    int btnH = 28;
    
    tft.fillRoundRect(10, btnY, 90, btnH, 5, COLOR_DANGER);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Cancel", 55, btnY + btnH/2);
    
    tft.fillRoundRect(110, btnY, 90, btnH, 5, COLOR_SECONDARY);
    tft.drawString("OK", 155, btnY + btnH/2);
}

void OnScreenKeyboard::drawKey(int row, int col, bool pressed) {
    int x = col * (KEY_WIDTH + KEY_SPACING) + 5;
    int y = row * (KEY_HEIGHT + KEY_SPACING) + KEYBOARD_START_Y;
    
    char key = KEYBOARD_LAYOUT[row][col];
    
    // Special color for backspace and done
    uint16_t color = COLOR_BUTTON;
    if (key == '<') color = COLOR_WARNING;  // Backspace
    if (key == '>') color = COLOR_SECONDARY; // Done
    if (pressed) color = COLOR_BUTTON_PRESSED;
    
    tft.fillRoundRect(x, y, KEY_WIDTH, KEY_HEIGHT, 3, color);
    
    // Draw character
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    
    // Center text (TFT_eSPI style)
    int textX = x + KEY_WIDTH / 2;
    int textY = y + KEY_HEIGHT / 2;
    tft.setTextDatum(MC_DATUM);
    
    // Special symbols
    if (key == '<') {
        tft.drawString("<", textX, textY);  // Backspace symbol
    } else if (key == '>') {
        tft.setTextSize(1);
        tft.drawString("OK", textX, textY);
    } else if (key == ' ') {
        tft.setTextSize(1);
        tft.drawString("Space", textX, textY);
    } else {
        tft.drawChar(key, textX, textY);
    }
}

void OnScreenKeyboard::drawTextArea() {
    // Clear text area
    tft.fillRect(0, 0, SCREEN_WIDTH, KEYBOARD_START_Y - 2, BACKGROUND_COLOR);
    
    // Prompt (kompakter)
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 3);
    tft.print(promptText);
    
    // Text box (kompakter)
    tft.fillRoundRect(10, 20, SCREEN_WIDTH - 20, 35, 5, COLOR_BUTTON);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(15, 32);
    
    if (currentText.length() > 0) {
        // Show only last N characters if too long
        if (currentText.length() > 18) {
            tft.print(currentText.substring(currentText.length() - 18));
        } else {
            tft.print(currentText);
        }
    } else {
        tft.setTextColor(TFT_DARKGREY);
        tft.print("...");
    }
    
    // Character count
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor(SCREEN_WIDTH - 40, 62);
    tft.printf("%u/%u", currentText.length(), maxLen);
}

void OnScreenKeyboard::drawNumberPicker() {
    tft.fillScreen(BACKGROUND_COLOR);
    
    // Prompt
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print(promptText);
    
    int centerX = SCREEN_WIDTH / 2;
    int centerY = 120;
    
    // Number display
    tft.fillRoundRect(centerX - 60, centerY - 25, 120, 50, 5, COLOR_BUTTON);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(3);
    
    String numStr = String(currentNumber);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(numStr, centerX, centerY);
    
    // Plus buttons (right side)
    tft.fillRoundRect(centerX + 60, centerY - 60, 50, 50, 5, COLOR_SECONDARY);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(centerX + 72, centerY - 50);
    tft.print("+10");
    tft.setTextSize(2);
    
    tft.fillRoundRect(centerX + 60, centerY, 50, 50, 5, COLOR_SECONDARY);
    tft.setTextSize(3);
    tft.setCursor(centerX + 77, centerY + 10);
    tft.print("+1");
    
    // Minus buttons (left side)
    tft.fillRoundRect(centerX - 110, centerY - 60, 50, 50, 5, COLOR_WARNING);
    tft.setTextSize(2);
    tft.setCursor(centerX - 103, centerY - 50);
    tft.print("-10");
    
    tft.fillRoundRect(centerX - 110, centerY, 50, 50, 5, COLOR_WARNING);
    tft.setTextSize(3);
    tft.setCursor(centerX - 95, centerY + 10);
    tft.print("-1");
    
    // Range info
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor(10, centerY + 80);
    tft.printf("Min: %ld | Max: %ld", minValue, maxValue);
    
    // Control buttons
    tft.fillRoundRect(10, SCREEN_HEIGHT - 50, 100, 40, 5, COLOR_DANGER);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(25, SCREEN_HEIGHT - 38);
    tft.print("Cancel");
    
    tft.fillRoundRect(120, SCREEN_HEIGHT - 50, 100, 40, 5, COLOR_SECONDARY);
    tft.setCursor(145, SCREEN_HEIGHT - 38);
    tft.print("OK");
}

// ============================================================
// Touch Handling
// ============================================================

bool OnScreenKeyboard::getTouchKey(uint16_t x, uint16_t y, int& row, int& col) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 10; c++) {
            if (isTouchInKey(x, y, r, c)) {
                row = r;
                col = c;
                
                // Visual feedback
                drawKey(r, c, true);
                delay(100);
                drawKey(r, c, false);
                
                return true;
            }
        }
    }
    return false;
}

bool OnScreenKeyboard::isTouchInKey(uint16_t x, uint16_t y, int row, int col) {
    int keyX = col * (KEY_WIDTH + KEY_SPACING) + 5;
    int keyY = row * (KEY_HEIGHT + KEY_SPACING) + KEYBOARD_START_Y;
    
    return x >= keyX && x <= (keyX + KEY_WIDTH) && 
           y >= keyY && y <= (keyY + KEY_HEIGHT);
}

void OnScreenKeyboard::handleBackspace() {
    if (currentText.length() > 0) {
        currentText.remove(currentText.length() - 1);
        drawTextArea();
    }
}

void OnScreenKeyboard::handleDone() {
    done = true;
}

void OnScreenKeyboard::handleCancel() {
    cancelled = true;
}

bool OnScreenKeyboard::isTouchInRect(uint16_t tx, uint16_t ty, int x, int y, int w, int h) {
    return tx >= x && tx <= (x + w) && ty >= y && ty <= (y + h);
}

