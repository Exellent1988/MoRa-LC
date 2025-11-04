#include "base_screen.h"
#include "../../hardware/display.h"
#include "../../hardware/touch.h"

BaseScreen::BaseScreen() : _display(nullptr), _touch(nullptr), _needsRedraw(true) {
}

BaseScreen::~BaseScreen() {
}

void BaseScreen::drawHeader(const String& title, bool showBack) {
    if (!_display) {
        Serial.println("[BaseScreen] ERROR: No display in drawHeader!");
        return;
    }
    
    LGFX& lcd = _display->getLGFX();
    
    // Header background
    lcd.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, Colors::HEADER_BG);
    
    // Back button
    if (showBack) {
        int btnW = 45;
        int btnH = 30;
        int btnX = 5;
        int btnY = (HEADER_HEIGHT - btnH) / 2;
        lcd.fillRoundRect(btnX, btnY, btnW, btnH, 5, Colors::BUTTON);
        lcd.drawRoundRect(btnX, btnY, btnW, btnH, 5, Colors::BORDER);
        lcd.setTextColor(Colors::BUTTON_TEXT);
        lcd.setTextSize(Typography::LARGE);
        lcd.setTextDatum(MC_DATUM);
        lcd.drawString("<", btnX + btnW/2, btnY + btnH/2);
    }
    
    // Title
    lcd.setTextColor(Colors::HEADER_TEXT);
    lcd.setTextSize(Typography::NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(title, SCREEN_WIDTH / 2, HEADER_HEIGHT / 2);
}

void BaseScreen::drawFooter() {
    // Footer can be implemented per screen if needed
}

