#include "ble_test_screen.h"
#include "../theme.h"
#include "../navigation.h"
#include "../../hardware/display.h"
#include "../../core/config.h"

BLETestScreen::BLETestScreen(BeaconService* beaconService)
    : _beaconService(beaconService)
    , _navigation(nullptr)
    , _lastUpdate(0)
    , _scrollOffset(0) {
}

BLETestScreen::~BLETestScreen() {
}

void BLETestScreen::onEnter() {
    Serial.println("[BLETest] Screen entered");
    
    // Ensure BLE is scanning
    if (_beaconService && !_beaconService->isScanning()) {
        Serial.println("[BLETest] Starting BLE scan...");
        _beaconService->startScanning(0);
    }
    _lastUpdate = millis();
    setNeedsRedraw(true);  // Force initial draw
}

void BLETestScreen::draw() {
    if (!_display) {
        Serial.println("[BLETest] ERROR: No display!");
        return;
    }
    
    Serial.println("[BLETest] Drawing screen...");
    
    LGFX& lcd = _display->getLGFX();
    
    // Clear screen
    lcd.fillScreen(Colors::BACKGROUND);
    
    // Draw header
    drawHeader("BLE Test", true);
    
    // Draw info
    int y = HEADER_HEIGHT + Spacing::SM;
    lcd.setTextColor(Colors::TEXT);
    lcd.setTextSize(Typography::NORMAL);
    lcd.setCursor(Spacing::MD, y);
    
    if (_beaconService && _beaconService->isScanning()) {
        lcd.setTextColor(Colors::SUCCESS);
        lcd.print("Scanning...");
    } else {
        lcd.setTextColor(Colors::WARNING);
        lcd.print("Not Scanning!");
    }
    
    y += 20;
    
    // Draw beacon count
    lcd.setTextColor(Colors::TEXT);
    lcd.setCursor(Spacing::MD, y);
    lcd.printf("Beacons: %d", (int)_beacons.size());
    
    Serial.printf("[BLETest] Drawing: Scanning=%d, Beacons=%d\n", 
                 _beaconService && _beaconService->isScanning(), 
                 (int)_beacons.size());
    
    y += Spacing::MD + 5;
    
    // Draw separator
    lcd.drawLine(0, y, SCREEN_WIDTH, y, Colors::BORDER);
    y += Spacing::SM;
    
    // Draw beacon list
    drawBeaconList();
}

void BLETestScreen::drawBeaconList() {
    if (!_display) return;
    
    LGFX& lcd = _display->getLGFX();
    
    int y = HEADER_HEIGHT + Spacing::MD * 3 + 5 + Spacing::SM;
    int itemHeight = Sizes::LIST_ITEM_H;
    int visibleItems = (SCREEN_HEIGHT - y) / itemHeight;
    
    // Calculate which items to show
    int startIdx = _scrollOffset;
    int endIdx = startIdx + visibleItems;
    if (endIdx > (int)_beacons.size()) {
        endIdx = _beacons.size();
    }
    
    // Draw visible beacons
    for (int i = startIdx; i < endIdx; i++) {
        if (i < (int)_beacons.size()) {
            drawBeaconItem(i, _beacons[i], y);
        }
        y += itemHeight;
    }
    
    // Draw scroll indicators
    if (_scrollOffset > 0) {
        // Up arrow
        lcd.fillTriangle(SCREEN_WIDTH - 15, HEADER_HEIGHT + 10,
                        SCREEN_WIDTH - 5, HEADER_HEIGHT + 10,
                        SCREEN_WIDTH - 10, HEADER_HEIGHT + 5,
                        Colors::BUTTON);
    }
    
    if (_scrollOffset + visibleItems < (int)_beacons.size()) {
        // Down arrow
        int arrowY = SCREEN_HEIGHT - 15;
        lcd.fillTriangle(SCREEN_WIDTH - 15, arrowY + 5,
                        SCREEN_WIDTH - 5, arrowY + 5,
                        SCREEN_WIDTH - 10, arrowY + 10,
                        Colors::BUTTON);
    }
}

void BLETestScreen::drawBeaconItem(int index, const BeaconInfo& beacon, int y) {
    if (!_display) return;
    
    LGFX& lcd = _display->getLGFX();
    
    // Background
    uint16_t bgColor = (index % 2 == 0) ? Colors::SURFACE : Colors::LIST_ITEM;
    lcd.fillRect(0, y, SCREEN_WIDTH, Sizes::LIST_ITEM_H, bgColor);
    
    // MAC address (safely)
    lcd.setTextColor(Colors::TEXT);
    lcd.setTextSize(Typography::SMALL);
    lcd.setCursor(Spacing::SM, y + 5);
    // Use c_str() to avoid String operations
    const char* macStr = beacon.macAddress.c_str();
    if (macStr) {
        lcd.print(macStr);
    }
    
    // RSSI
    lcd.setCursor(Spacing::SM, y + 20);
    lcd.setTextColor(beacon.rssi > -70 ? Colors::SUCCESS : 
                     beacon.rssi > -85 ? Colors::WARNING : Colors::TEXT);
    lcd.printf("RSSI: %d (avg: %d)", beacon.rssi, beacon.avgRssi);
    
    // UUID (if available, truncated) - use c_str() safely
    const char* uuidStr = beacon.uuid.c_str();
    if (uuidStr && strlen(uuidStr) > 0 && strcmp(uuidStr, macStr) != 0) {
        lcd.setTextColor(Colors::TEXT_SECONDARY);
        lcd.setTextSize(Typography::TINY);
        lcd.setCursor(SCREEN_WIDTH - 120, y + 5);
        // Print first 8 chars only
        char uuidShort[12];
        strncpy(uuidShort, uuidStr, 8);
        uuidShort[8] = '\0';
        strcat(uuidShort, "...");
        lcd.print(uuidShort);
    }
    
    // Major/Minor (if iBeacon)
    if (beacon.major > 0 || beacon.minor > 0) {
        lcd.setTextColor(Colors::TEXT_SECONDARY);
        lcd.setTextSize(Typography::TINY);
        lcd.setCursor(SCREEN_WIDTH - 60, y + 20);
        lcd.printf("%u/%u", beacon.major, beacon.minor);
    }
}

void BLETestScreen::update(uint32_t deltaTime) {
    // Update beacon list every 500ms
    uint32_t now = millis();
    if (now - _lastUpdate > 500) {
        if (_beaconService) {
            try {
                _beacons = _beaconService->getBeacons();
                
                // Limit scroll offset
                int maxScroll = (int)_beacons.size() - 3;  // Show 3 items
                if (maxScroll < 0) maxScroll = 0;
                if (_scrollOffset > maxScroll) {
                    _scrollOffset = maxScroll;
                }
            } catch (...) {
                // Ignore errors - just skip this update
                Serial.println("[BLETest] ERROR: Exception in update()");
            }
        }
        _lastUpdate = now;
        setNeedsRedraw(true);
    }
}

bool BLETestScreen::handleTouch(uint16_t x, uint16_t y) {
    // Back button (top left) - Navigate back
    if (x < 55 && y < HEADER_HEIGHT) {
        Serial.println("[BLETest] Back button pressed");
        if (_navigation) {
            _navigation->goBack();
        }
        return true;
    }
    
    // Scroll up (top right)
    if (x > SCREEN_WIDTH - 30 && y < HEADER_HEIGHT + 30 && _scrollOffset > 0) {
        _scrollOffset--;
        setNeedsRedraw(true);
        return true;
    }
    
    // Scroll down (bottom right)
    if (x > SCREEN_WIDTH - 30 && y > SCREEN_HEIGHT - 30 && 
        _scrollOffset < (int)_beacons.size() - 3) {
        _scrollOffset++;
        setNeedsRedraw(true);
        return true;
    }
    
    return false;
}

