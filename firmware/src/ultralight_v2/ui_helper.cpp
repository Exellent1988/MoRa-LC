#include "ui_helper.h"
#include "ui_state.h"
#include "display_manager.h"
#include "ui_keyboard.h"

extern DisplayManager display;
extern UIState uiState;

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
    
    // Simple message like old variant - no OK button, just delay
    lcd.fillScreen(BACKGROUND_COLOR);
    lcd.setTextColor(color);
    lcd.setTextSize(TEXT_SIZE_NORMAL);  // Smaller: LARGE -> NORMAL
    
    lcd.setTextDatum(TC_DATUM);  // Top Center
    lcd.drawString(title, SCREEN_WIDTH / 2, 60);  // Higher: 80 -> 60
    
    lcd.setTextSize(1);  // Even smaller for message
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextDatum(TC_DATUM);
    lcd.drawString(message, SCREEN_WIDTH / 2, 100);  // Higher: 120 -> 100
    
    delay(2000);  // Show for 2 seconds like old variant
    uiState.needsRedraw = true;
}

String inputText(const String& prompt, const String& defaultValue) {
    OnScreenKeyboard keyboard(display);
    String result = keyboard.getText(prompt, defaultValue, 20);
    uiState.needsRedraw = true;
    return result;
}

uint32_t inputNumber(const String& prompt, uint32_t defaultValue, uint32_t min, uint32_t max) {
    OnScreenKeyboard keyboard(display);
    uint32_t result = keyboard.getNumber(prompt, defaultValue, min, max);
    uiState.needsRedraw = true;
    return result;
}

// Format beacon MAC address: c3:00:00:42:d9:2b -> 42:d9:2b (last 4 digits)
String formatBeaconMAC(const String& macAddress) {
    // Check if it's a c3:00:00:XX:XX:XX beacon
    if (macAddress.startsWith("c3:00:00:")) {
        // Return only the last 8 characters (XX:XX:XX)
        if (macAddress.length() >= 8) {
            return macAddress.substring(macAddress.length() - 8);
        }
    }
    // Otherwise return full MAC
    return macAddress;
}
