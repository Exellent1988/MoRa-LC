#include "home_screen.h"
#include "../theme.h"
#include "../../hardware/display.h"

HomeScreen::HomeScreen() {
}

HomeScreen::~HomeScreen() {
}

void HomeScreen::draw() {
    if (!_display) {
        Serial.println("[HomeScreen] ERROR: No display!");
        return;
    }
    
    Serial.println("[HomeScreen] Drawing screen...");
    
    LGFX& lcd = _display->getLGFX();
    
    // Clear screen
    lcd.fillScreen(Colors::BACKGROUND);
    
    // Draw header
    drawHeader("MoRa-LC", false);
    
    Serial.println("[HomeScreen] Header drawn");
    
    // Draw icon buttons (placeholder - icons will be added later)
    int y = HEADER_HEIGHT + Spacing::MD;
    int btnW = SCREEN_WIDTH - 2 * Spacing::MD;
    int btnH = Sizes::BUTTON_H;
    int btnSpacing = Spacing::SM;
    
    // Race button
    drawIconButton(Spacing::MD, y, btnW, btnH, "Neues Rennen", ICON_RACE, Colors::SECONDARY);
    y += btnH + btnSpacing;
    
    // Teams button
    drawIconButton(Spacing::MD, y, btnW, btnH, "Teams", ICON_TEAM, Colors::BUTTON);
    y += btnH + btnSpacing;
    
    // Results button
    drawIconButton(Spacing::MD, y, btnW, btnH, "Ergebnisse", ICON_RESULTS, Colors::BUTTON);
    y += btnH + btnSpacing;
    
    // Settings button
    drawIconButton(Spacing::MD, y, btnW, btnH, "Einstellungen", ICON_SETTINGS, Colors::BUTTON);
}

void HomeScreen::drawIconButton(int x, int y, int w, int h, const String& label, IconID icon, uint16_t color) {
    if (!_display) return;
    
    LGFX& lcd = _display->getLGFX();
    
    // Button background
    lcd.fillRoundRect(x, y, w, h, 8, color);
    lcd.drawRoundRect(x, y, w, h, 8, Colors::BORDER);
    
    // TODO: Draw icon (when icon system is implemented)
    // For now, just text
    lcd.setTextColor(Colors::BUTTON_TEXT);
    lcd.setTextSize(Typography::NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(label, x + w / 2, y + h / 2);
}

bool HomeScreen::handleTouch(uint16_t x, uint16_t y) {
    // Debug: Print touch coordinates
    Serial.printf("[HomeScreen] Touch at x=%u, y=%u\n", x, y);
    
    // TODO: Implement touch handling for buttons
    // Calculate button positions and check if touched
    
    int yPos = HEADER_HEIGHT + Spacing::MD;
    int btnW = SCREEN_WIDTH - 2 * Spacing::MD;
    int btnH = Sizes::BUTTON_H;
    int btnSpacing = Spacing::SM;
    
    // Check each button
    // Button 1: "Neues Rennen" (yPos)
    if (x >= Spacing::MD && x < Spacing::MD + btnW &&
        y >= yPos && y < yPos + btnH) {
        Serial.println("[HomeScreen] 'Neues Rennen' button pressed");
        // TODO: Navigate to race setup screen
        return true;
    }
    
    // Button 2: "Teams" (yPos + btnH + btnSpacing)
    yPos += btnH + btnSpacing;
    if (x >= Spacing::MD && x < Spacing::MD + btnW &&
        y >= yPos && y < yPos + btnH) {
        Serial.println("[HomeScreen] 'Teams' button pressed");
        // TODO: Navigate to teams screen
        return true;
    }
    
    // Button 3: "Ergebnisse" (yPos + btnH + btnSpacing)
    yPos += btnH + btnSpacing;
    if (x >= Spacing::MD && x < Spacing::MD + btnW &&
        y >= yPos && y < yPos + btnH) {
        Serial.println("[HomeScreen] 'Ergebnisse' button pressed");
        // TODO: Navigate to results screen
        return true;
    }
    
    // Button 4: "Einstellungen" (yPos + btnH + btnSpacing)
    yPos += btnH + btnSpacing;
    if (x >= Spacing::MD && x < Spacing::MD + btnW &&
        y >= yPos && y < yPos + btnH) {
        Serial.println("[HomeScreen] 'Einstellungen' button pressed");
        // TODO: Navigate to settings screen
        return true;
    }
    
    // Temporary: Add BLE Test button (long press on "Einstellungen" or double tap)
    // For now, we'll add it as a separate test button later
    
    return false;
}

