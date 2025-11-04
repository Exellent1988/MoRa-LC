#include "display.h"

Display::Display() : _initialized(false) {
}

Display::~Display() {
}

bool Display::begin() {
    configureLGFX();
    
    _lgfx.init();
    _lgfx.setRotation(SCREEN_ROTATION);
    
    // Backlight on
    pinMode(TFT_BL_PIN, OUTPUT);
    digitalWrite(TFT_BL_PIN, HIGH);
    
    // Load touch calibration data if available
    #if TOUCH_CAL_DATA_ENABLED
    Serial.println("[Display] Loading touch calibration data...");
    _lgfx.setTouchCalibrate(const_cast<uint16_t*>(TOUCH_CAL_DATA));
    Serial.println("[Display] Touch calibration loaded");
    #else
    Serial.println("[Display] No touch calibration data - using defaults");
    #endif
    
    _initialized = true;
    Serial.println("[Display] Initialized successfully");
    return true;
}

void Display::configureLGFX() {
    // LGFX is configured in constructor via LGFX_Config.hpp
    // This method is kept for future extensibility
}

void Display::fillScreen(uint16_t color) {
    _lgfx.fillScreen(color);
}

void Display::setTextColor(uint16_t color) {
    _lgfx.setTextColor(color);
}

void Display::setTextSize(uint8_t size) {
    _lgfx.setTextSize(size);
}

void Display::setCursor(int16_t x, int16_t y) {
    _lgfx.setCursor(x, y);
}

void Display::print(const String& text) {
    _lgfx.print(text);
}

void Display::println(const String& text) {
    _lgfx.println(text);
}

bool Display::getTouch(uint16_t* x, uint16_t* y) {
    if (!_initialized) return false;
    
    // LovyanGFX getTouch() returns calibrated coordinates directly
    // No need for manual calibration mapping if setTouchCalibrate() was called
    uint16_t tx, ty;
    if (_lgfx.getTouch(&tx, &ty)) {
        // Clamp to screen bounds (safety check)
        tx = constrain(tx, 0, SCREEN_WIDTH - 1);
        ty = constrain(ty, 0, SCREEN_HEIGHT - 1);
        
        *x = tx;
        *y = ty;
        
        // Debug output (uncomment for testing)
        // Serial.printf("[Touch] x=%u, y=%u\n", *x, *y);
        
        return true;
    }
    return false;
}

bool Display::isTouched() {
    if (!_initialized) return false;
    int16_t x, y;
    return _lgfx.getTouch(&x, &y);
}

