#include "ui_keyboard.h"

OnScreenKeyboard::OnScreenKeyboard(DisplayManager& disp) 
    : display(disp), currentText(""), promptText(""), maxLen(20), done(false), cancelled(false),
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
        if (display.getTouch(&x, &y)) {
            // Debounce
            delay(50);
            while (display.getTouch(&x, &y)) {
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
        if (display.getTouch(&x, &y)) {
            // Debounce
            delay(50);
            while (display.getTouch(&x, &y)) {
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
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    
    // Draw all keys
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 10; col++) {
            drawKey(row, col, false);
        }
    }
    
    // Draw control buttons
    int btnY = 190;
    int btnH = 28;
    
    lcd.fillRoundRect(10, btnY, 90, btnH, 5, COLOR_DANGER);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("Cancel", 55, btnY + btnH/2);
    
    lcd.fillRoundRect(110, btnY, 90, btnH, 5, COLOR_SECONDARY);
    lcd.drawString("OK", 155, btnY + btnH/2);
}

void OnScreenKeyboard::drawKey(int row, int col, bool pressed) {
    LGFX& lcd = display.getDisplay();
    int x = col * (KEY_WIDTH + KEY_SPACING) + 5;
    int y = row * (KEY_HEIGHT + KEY_SPACING) + KEYBOARD_START_Y;
    
    char key = KEYBOARD_LAYOUT[row][col];
    
    // Special color for backspace and done
    uint16_t color = COLOR_BUTTON;
    if (key == '<') color = COLOR_WARNING;  // Backspace
    if (key == '>') color = COLOR_SECONDARY; // Done
    if (pressed) color = COLOR_BUTTON_PRESSED;
    
    lcd.fillRoundRect(x, y, KEY_WIDTH, KEY_HEIGHT, 3, color);
    
    // Draw character
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    
    // Center text
    int textX = x + KEY_WIDTH / 2;
    int textY = y + KEY_HEIGHT / 2;
    lcd.setTextDatum(MC_DATUM);
    
    // Special symbols
    if (key == '<') {
        lcd.drawString("<", textX, textY);  // Backspace symbol
    } else if (key == '>') {
        lcd.setTextSize(1);
        lcd.drawString("OK", textX, textY);
    } else if (key == ' ') {
        lcd.setTextSize(1);
        lcd.drawString("Space", textX, textY);
    } else {
        char buf[2] = {key, '\0'};
        lcd.drawString(buf, textX, textY);
    }
}

void OnScreenKeyboard::drawTextArea() {
    LGFX& lcd = display.getDisplay();
    
    // Clear text area
    lcd.fillRect(0, 0, SCREEN_WIDTH, KEYBOARD_START_Y - 2, BACKGROUND_COLOR);
    
    // Prompt
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, 3);
    lcd.print(promptText);
    
    // Text box
    lcd.fillRoundRect(10, 20, SCREEN_WIDTH - 20, 35, 5, 0xFFFF);  // White
    lcd.setTextColor(0x0000);  // Black text
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(15, 32);
    
    if (currentText.length() > 0) {
        // Show only last N characters if too long
        if (currentText.length() > 18) {
            lcd.print(currentText.substring(currentText.length() - 18));
        } else {
            lcd.print(currentText);
        }
    } else {
        lcd.setTextColor(0x630C);  // Gray
        lcd.print("...");
    }
    
    // Character count
    lcd.setTextSize(1);
    lcd.setTextColor(0x630C);
    lcd.setCursor(SCREEN_WIDTH - 40, 62);
    lcd.printf("%u/%u", currentText.length(), maxLen);
}

void OnScreenKeyboard::drawNumberPicker() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    
    // Prompt
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_LARGE);
    lcd.setCursor(10, 10);
    lcd.print(promptText);
    
    // Number display
    int centerX = SCREEN_WIDTH / 2;
    int centerY = 120;
    
    lcd.fillRoundRect(centerX - 50, centerY - 30, 100, 60, 10, COLOR_SECONDARY);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(4);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(String(currentNumber), centerX, centerY);
    
    // Plus buttons (right)
    lcd.fillRoundRect(centerX + 60, centerY - 60, 50, 50, 5, COLOR_BUTTON);
    lcd.setTextSize(TEXT_SIZE_LARGE);
    lcd.drawString("+10", centerX + 85, centerY - 35);
    
    lcd.fillRoundRect(centerX + 60, centerY, 50, 50, 5, COLOR_BUTTON);
    lcd.drawString("+1", centerX + 85, centerY + 25);
    
    // Minus buttons (left)
    lcd.fillRoundRect(centerX - 110, centerY - 60, 50, 50, 5, COLOR_BUTTON);
    lcd.drawString("-10", centerX - 85, centerY - 35);
    
    lcd.fillRoundRect(centerX - 110, centerY, 50, 50, 5, COLOR_BUTTON);
    lcd.drawString("-1", centerX - 85, centerY + 25);
    
    // Control buttons (bottom)
    lcd.fillRoundRect(10, SCREEN_HEIGHT - 50, 100, 40, 5, COLOR_DANGER);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.drawString("Cancel", 60, SCREEN_HEIGHT - 30);
    
    lcd.fillRoundRect(120, SCREEN_HEIGHT - 50, 100, 40, 5, COLOR_SECONDARY);
    lcd.drawString("OK", 170, SCREEN_HEIGHT - 30);
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

