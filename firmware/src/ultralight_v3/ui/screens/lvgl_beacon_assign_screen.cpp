#include "lvgl_beacon_assign_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLBeaconAssignScreen::LVGLBeaconAssignScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _beaconService(beaconService)
    , _lapCounter(lapCounter)
    , _teamId(0)
    , _instructionLabel(nullptr)
    , _beaconList(nullptr)
    , _lastUpdate(0) {
}

LVGLBeaconAssignScreen::~LVGLBeaconAssignScreen() {
}

void LVGLBeaconAssignScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLBeaconAssign] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLBeaconAssign] Creating beacon assign screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header with back button
    createHeader("Beacon zuordnen", true, backBtnEventHandler, this);
    
    // Instruction
    TeamData* team = _lapCounter ? _lapCounter->getTeam(_teamId) : nullptr;
    String instruction = "Beacon für Team auswählen";
    if (team) {
        instruction = "Beacon für: " + team->teamName;
    }
    
    _instructionLabel = createLabel(instruction.c_str(), Spacing::MD, HEADER_HEIGHT + Spacing::SM,
                                   SCREEN_WIDTH - 2 * Spacing::MD, 30, rgb565ToLVGL(Colors::TEXT));
    
    // Create beacon list
    int listY = HEADER_HEIGHT + Spacing::MD + 30;
    int listH = SCREEN_HEIGHT - listY - Spacing::MD;
    _beaconList = createList(Spacing::MD, listY, SCREEN_WIDTH - 2 * Spacing::MD, listH);
    
    // Ensure BLE is scanning
    if (_beaconService && !_beaconService->isScanning()) {
        _beaconService->startScanning(0);
    }
    
    _lastUpdate = millis();
    updateBeaconList();
    
    Serial.println("[LVGLBeaconAssign] Beacon assign screen created");
}

void LVGLBeaconAssignScreen::onExit() {
    // Cleanup if needed
}

void LVGLBeaconAssignScreen::update() {
    if (!_beaconList || !_beaconService) return;
    
    // Update beacon list every 500ms
    uint32_t now = millis();
    if (now - _lastUpdate > 500) {
        updateBeaconList();
        _lastUpdate = now;
    }
}

void LVGLBeaconAssignScreen::updateBeaconList() {
    if (!_beaconList || !_beaconService) return;
    
    // Clear existing list items
    lv_obj_clean(_beaconList);
    
    // Get beacons
    std::vector<BeaconInfo> beacons = _beaconService->getBeacons();
    
    if (beacons.empty()) {
        lv_obj_t* item = lv_list_add_btn(_beaconList, LV_SYMBOL_BLUETOOTH, "Keine Beacons gefunden");
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    // Add beacons to list
    for (const auto& beacon : beacons) {
        char info[128];
        snprintf(info, sizeof(info), "%s\nRSSI: %d dBm", beacon.macAddress.c_str(), beacon.avgRssi);
        
        lv_obj_t* item = lv_list_add_btn(_beaconList, LV_SYMBOL_BLUETOOTH, info);
        lv_obj_add_event_cb(item, beaconItemEventHandler, LV_EVENT_CLICKED, this);
        
        // Store beacon UUID in user data (we'll use a simple string copy)
        // For now, use the label text to identify the beacon
    }
}

void LVGLBeaconAssignScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLBeaconAssignScreen* screen = (LVGLBeaconAssignScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLBeaconAssign] Back button clicked");
        screen->_navigation->goBack();
    }
}

void LVGLBeaconAssignScreen::beaconItemEventHandler(lv_event_t* e) {
    LVGLBeaconAssignScreen* screen = (LVGLBeaconAssignScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_lapCounter) return;
    
    // Get beacon UUID from list item (we'll use the label text)
    lv_obj_t* item = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(item, 1); // Second child is usually the label
    if (label) {
        const char* text = lv_label_get_text(label);
        // Extract MAC address from text (first line before \n)
        String macAddress = String(text);
        int newlinePos = macAddress.indexOf('\n');
        if (newlinePos > 0) {
            macAddress = macAddress.substring(0, newlinePos);
        }
        
        screen->assignBeaconToTeam(macAddress);
    }
}

void LVGLBeaconAssignScreen::assignBeaconToTeam(const String& beaconUUID) {
    if (!_lapCounter || _teamId == 0) return;
    
    if (_lapCounter->assignBeacon(_teamId, beaconUUID)) {
        Serial.printf("[LVGLBeaconAssign] Beacon %s assigned to team %u\n", beaconUUID.c_str(), _teamId);
        
        // Navigate back
        if (_navigation) {
            _navigation->goBack();
        }
    }
}

