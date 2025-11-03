#include "ui_helper.h"
#include "display_manager.h"

extern DisplayManager display;

void drawHeader(const String& title, bool showBack) {
    LGFX& lcd = display.getDisplay();
    
    // Header background - dunkler für Kontrast
    lcd.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_HEADER_BG);
    
    // Back button - größer und besser sichtbar
    if (showBack) {
        int btnW = 40;
        int btnH = 30;
        lcd.fillRoundRect(5, 5, btnW, btnH, 5, COLOR_BUTTON);
        lcd.setTextColor(COLOR_BUTTON_TEXT);
        lcd.setTextSize(TEXT_SIZE_LARGE);
        lcd.setTextDatum(MC_DATUM);
        lcd.drawString("<", 5 + btnW/2, 5 + btnH/2);
    }
    
    // Title - größer und weiß auf dunklem Hintergrund
    lcd.setTextColor(COLOR_HEADER_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(title, SCREEN_WIDTH / 2, HEADER_HEIGHT / 2);
}

void drawButton(int x, int y, int w, int h, const String& text, uint16_t color) {
    LGFX& lcd = display.getDisplay();
    
    // Button background with rounded corners
    lcd.fillRoundRect(x, y, w, h, 8, color);
    
    // Button border - dunkler für Kontrast auf hellem Hintergrund
    lcd.drawRoundRect(x, y, w, h, 8, 0x4208);  // Dark gray border
    
    // Button text - schwarz auf hellem Button-Hintergrund
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);  // Size 2 for readability
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(text, x + w / 2, y + h / 2);
}

bool isTouchInRect(uint16_t tx, uint16_t ty, int x, int y, int w, int h) {
    return (tx >= x && tx < (x + w) && ty >= y && ty < (y + h));
}

void showMessage(const String& title, const String& message, uint16_t color) {
    LGFX& lcd = display.getDisplay();
    
    // Message box background - größer und besser lesbar
    int boxW = SCREEN_WIDTH - 40;
    int boxH = 140;
    int boxX = 20;
    int boxY = (SCREEN_HEIGHT - boxH) / 2;
    
    // Background (weiß/hellgrau)
    lcd.fillRoundRect(boxX, boxY, boxW, boxH, 10, 0xFFFF);  // White background
    lcd.drawRoundRect(boxX, boxY, boxW, boxH, 10, color);
    
    // Title - größer und dunkel
    lcd.setTextColor(color);
    lcd.setTextSize(TEXT_SIZE_LARGE);
    lcd.setTextDatum(TC_DATUM);
    lcd.drawString(title, SCREEN_WIDTH / 2, boxY + 20);
    
    // Message - dunkler Text, größer
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(TC_DATUM);
    lcd.drawString(message, SCREEN_WIDTH / 2, boxY + 60);
    
    // OK button - größer
    drawButton(boxX + boxW / 2 - 50, boxY + boxH - 40, 100, 35, "OK", COLOR_SECONDARY);
}

