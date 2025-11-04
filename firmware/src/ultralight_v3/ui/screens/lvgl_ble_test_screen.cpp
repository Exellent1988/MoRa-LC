#include "lvgl_ble_test_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>
#include <cstring>

LVGLBLETestScreen::LVGLBLETestScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService)
    : LVGLBaseScreen(lvglDisplay)
    , _beaconService(beaconService)
    , _navigation(nullptr)
    , _list(nullptr)
    , _statusLabel(nullptr)
    , _lastUpdate(0) {
}

LVGLBLETestScreen::~LVGLBLETestScreen() {
}

void LVGLBLETestScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLBLETest] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLBLETest] Creating BLE test screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header with back button
    createHeader("BLE Test", true);
    
    // Create status label
    _statusLabel = createLabel("Scanning...", Spacing::MD, HEADER_HEIGHT + Spacing::SM, 
                               SCREEN_WIDTH - 2 * Spacing::MD, 20, rgb565ToLVGL(Colors::TEXT));
    
    // Create list for beacons
    int listY = HEADER_HEIGHT + Spacing::MD + 20;
    int listH = SCREEN_HEIGHT - listY - Spacing::MD;
    _list = createList(Spacing::MD, listY, SCREEN_WIDTH - 2 * Spacing::MD, listH);
    
    // Ensure BLE is scanning
    if (_beaconService && !_beaconService->isScanning()) {
        Serial.println("[LVGLBLETest] Starting BLE scan...");
        _beaconService->startScanning(0);
    }
    
    _lastUpdate = millis();
    updateBeaconList();
    
    Serial.println("[LVGLBLETest] BLE test screen created");
}

void LVGLBLETestScreen::onExit() {
    // Cleanup if needed
}

void LVGLBLETestScreen::update() {
    if (!_beaconService || !_list) return;
    
    // Update beacon list every 500ms
    uint32_t now = millis();
    if (now - _lastUpdate > 500) {
        updateBeaconList();
        _lastUpdate = now;
    }
}

void LVGLBLETestScreen::updateBeaconList() {
    if (!_list || !_beaconService) return;
    
    // Get current beacons
    std::vector<BeaconInfo> beacons = _beaconService->getBeacons();
    
    // Update status label
    if (_statusLabel) {
        char status[64];
        snprintf(status, sizeof(status), "Beacons: %zu | Scanning: %s", 
                 beacons.size(), _beaconService->isScanning() ? "YES" : "NO");
        lv_label_set_text(_statusLabel, status);
    }
    
    // Clear existing list items
    lv_obj_clean(_list);
    
    // Add beacons to list
    for (const auto& beacon : beacons) {
        addBeaconToList(beacon);
    }
}

void LVGLBLETestScreen::addBeaconToList(const BeaconInfo& beacon) {
    if (!_list) return;
    
    // Create list item
    lv_obj_t* item = lv_list_add_btn(_list, LV_SYMBOL_BLUETOOTH, nullptr);
    
    // Create label with beacon info
    char info[128];
    snprintf(info, sizeof(info), "%s\nRSSI: %d dBm (avg: %d)", 
             beacon.macAddress.c_str(), beacon.rssi, beacon.avgRssi);
    
    lv_obj_t* label = lv_label_create(item);
    lv_label_set_text(label, info);
    lv_obj_set_style_text_color(label, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 40, 0);
}

void LVGLBLETestScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLBLETestScreen* screen = (LVGLBLETestScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLBLETest] Back button clicked");
        screen->_navigation->goBack();
    }
}

