#include "display_manager.h"

DisplayManager::DisplayManager() 
    : _initialized(false)
{
}

DisplayManager::~DisplayManager() {
}

bool DisplayManager::begin() {
    Serial.println("[Display] Initializing LovyanGFX...");
    
    // Initialize display
    _display.init();
    _display.setRotation(1);  // Landscape (320x240)
    
    // Backlight on
    pinMode(TFT_BL_PIN, OUTPUT);
    digitalWrite(TFT_BL_PIN, HIGH);
    
    // Load touch calibration data if available
    #if TOUCH_CAL_DATA_ENABLED
    Serial.println("[Display] Loading touch calibration data...");
    _display.setTouchCalibrate(const_cast<uint16_t*>(TOUCH_CAL_DATA));
    Serial.println("[Display] Touch calibration loaded");
    #else
    Serial.println("[Display] No touch calibration data - using defaults");
    #endif
    
    // Fill screen with background color
    _display.fillScreen(BACKGROUND_COLOR);
    
    _initialized = true;
    Serial.println("[Display] LovyanGFX initialized successfully");
    return true;
}

bool DisplayManager::getTouch(uint16_t* x, uint16_t* y) {
    if (!_initialized) return false;
    
    // LovyanGFX getTouch() returns calibrated coordinates
    // On this hardware, we need to transform coordinates to match screen layout
    uint16_t rawX, rawY;
    if (_display.getTouch(&rawX, &rawY)) {
        // Debug: Print raw coordinates
        // Serial.printf("[Touch Raw] rawX=%u, rawY=%u\n", rawX, rawY);
        
        // Based on user feedback: y=18 when touching top means rawY is already correct
        // BUT we need to check: If user touches top and gets y=18, that's wrong!
        // Top should be y=0 or close to it. y=18 means we're 18px from bottom after inversion.
        // So rawY must have been ~222 (240-18=222), which is bottom.
        
        // Let's try: NO inversion first, just use raw coordinates
        // If that doesn't work, we'll try swapping and/or inverting
        
        // Current assumption: raw coordinates from LovyanGFX are already in screen space
        // But might need adjustment for hardware orientation
        *x = rawX;
        *y = rawY;
        
        // Clamp to screen bounds
        *x = constrain(*x, 0, SCREEN_WIDTH - 1);
        *y = constrain(*y, 0, SCREEN_HEIGHT - 1);
        
        return true;
    }
    
    return false;
}

bool DisplayManager::isTouched() {
    if (!_initialized) return false;
    uint16_t x, y;
    return _display.getTouch(&x, &y);
}

bool DisplayManager::calibrateTouch() {
    if (!_initialized) return false;
    
    Serial.println("\n=================================");
    Serial.println("Touch Calibration");
    Serial.println("=================================");
    Serial.println("Touch the crosshair at each corner");
    Serial.println("when it appears on screen.");
    Serial.println("=================================\n");
    
    // LovyanGFX calibrate_touch function
    // Parameters: calibration array (output), foreground color, background color, crosshair size
    uint16_t calData[8];
    
    _display.fillScreen(BACKGROUND_COLOR);
    _display.setTextColor(TEXT_COLOR);
    _display.setTextSize(2);
    _display.setTextDatum(MC_DATUM);
    _display.drawString("Touch Calibration", SCREEN_WIDTH / 2, 50);
    _display.setTextSize(1);
    _display.drawString("Touch each corner", SCREEN_WIDTH / 2, 80);
    _display.drawString("when crosshair appears", SCREEN_WIDTH / 2, 95);
    
    delay(2000);
    
    // Run calibration (this will show crosshairs at corners)
    // Use the public template method calibrateTouch() which accepts RGB565 colors
    // It automatically converts colors internally using _write_conv.convert()
    // Template signature: calibrateTouch(uint16_t *parameters, const T& color_fg, const T& color_bg, uint8_t size = 10)
    _display.calibrateTouch(calData, TEXT_COLOR, BACKGROUND_COLOR, 15);
    
    // Print calibration data
    Serial.println("\n=== TOUCH CALIBRATION DATA ===");
    Serial.print("uint16_t calData[8] = { ");
    for (int i = 0; i < 8; i++) {
        Serial.print(calData[i]);
        if (i < 7) Serial.print(", ");
    }
    Serial.println(" };");
    Serial.println("=================================\n");
    
    // Save calibration to display (LovyanGFX uses setTouchCalibrate for uint16_t array)
    _display.setTouchCalibrate(calData);
    
    // Show success message
    _display.fillScreen(BACKGROUND_COLOR);
    _display.setTextColor(COLOR_SECONDARY);
    _display.setTextSize(2);
    _display.setTextDatum(MC_DATUM);
    _display.drawString("Calibration", SCREEN_WIDTH / 2, 100);
    _display.drawString("Complete!", SCREEN_WIDTH / 2, 125);
    _display.setTextSize(1);
    _display.setTextColor(TEXT_COLOR);
    _display.drawString("Check Serial Monitor", SCREEN_WIDTH / 2, 160);
    _display.drawString("for calibration data", SCREEN_WIDTH / 2, 175);
    
    delay(3000);
    _display.fillScreen(BACKGROUND_COLOR);
    
    Serial.println("[Touch] Calibration complete!");
    return true;
}

void DisplayManager::fillScreen(uint16_t color) {
    if (!_initialized) return;
    _display.fillScreen(color);
}

void DisplayManager::setTextColor(uint16_t color) {
    if (!_initialized) return;
    _display.setTextColor(color);
}

void DisplayManager::setTextSize(uint8_t size) {
    if (!_initialized) return;
    _display.setTextSize(size);
}

void DisplayManager::setCursor(int16_t x, int16_t y) {
    if (!_initialized) return;
    _display.setCursor(x, y);
}

void DisplayManager::print(const String& text) {
    if (!_initialized) return;
    _display.print(text);
}

void DisplayManager::println(const String& text) {
    if (!_initialized) return;
    _display.println(text);
}

void DisplayManager::fillCircle(int32_t x, int32_t y, int32_t r, uint16_t color) {
    if (!_initialized) return;
    _display.fillCircle(x, y, r, color);
}

