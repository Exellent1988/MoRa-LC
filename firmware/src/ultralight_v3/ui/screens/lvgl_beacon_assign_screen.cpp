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
    , _lastUpdate(0)
    , _macCount(0)
    , _lastBeaconCount(-1) {
}

LVGLBeaconAssignScreen::~LVGLBeaconAssignScreen() {
}

void LVGLBeaconAssignScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLBeaconAssign] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLBeaconAssign] Creating beacon assign screen...");
    
    _screen = _lvglDisplay->getScreen();
    lv_obj_clean(_screen);
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    createHeader("Assign Beacon", true, backBtnEventHandler, this);
    
    TeamData* team = _lapCounter ? _lapCounter->getTeam(_teamId) : nullptr;
    String instruction = "Select beacon for team";
    if (team) {
        instruction = "Beacon for: " + team->teamName;
    }
    
    _instructionLabel = createLabel(instruction.c_str(), Spacing::SM, HEADER_HEIGHT + Spacing::XS,
                                   SCREEN_WIDTH - 2 * Spacing::SM, 20, rgb565ToLVGL(Colors::TEXT));
    lv_obj_set_style_text_font(_instructionLabel, &lv_font_montserrat_14, 0);
    
    int listY = HEADER_HEIGHT + Spacing::XS + 22;
    int listH = SCREEN_HEIGHT - listY - Spacing::XS;
    _beaconList = createList(0, listY, SCREEN_WIDTH, listH);
    lv_obj_set_style_pad_row(_beaconList, 1, 0);
    
    if (_beaconService && !_beaconService->isScanning()) {
        _beaconService->startScanning(0);
    }
    
    _lastBeaconCount = -1;
    _lastUpdate = millis();
    rebuildBeaconList();
    
    Serial.println("[LVGLBeaconAssign] Beacon assign screen created");
}

void LVGLBeaconAssignScreen::onExit() {
}

void LVGLBeaconAssignScreen::update() {
    if (!_beaconList || !_beaconService) return;
    
    uint32_t now = millis();
    if (now - _lastUpdate > 2000) {
        updateBeaconList();
        _lastUpdate = now;
    }
}

static const char* macSuffix(const String& mac) {
    int len = mac.length();
    return (len > 5) ? mac.c_str() + len - 5 : mac.c_str();
}

String LVGLBeaconAssignScreen::getOwnerTeamName(const String& mac) const {
    if (!_lapCounter) return "";
    auto teams = _lapCounter->getAllTeams();
    for (auto* t : teams) {
        if (t->beaconUUID == mac) {
            return t->teamName;
        }
    }
    return "";
}

void LVGLBeaconAssignScreen::updateBeaconList() {
    if (!_beaconList || !_beaconService) return;
    
    std::vector<BeaconInfo> beacons = _beaconService->getBeacons();
    int count = (int)beacons.size();
    
    if (count != _lastBeaconCount) {
        rebuildBeaconList();
        return;
    }
    
    // Only update RSSI text on existing items (no rebuild, preserves scroll)
    uint32_t childCount = lv_obj_get_child_cnt(_beaconList);
    int idx = 0;
    for (const auto& beacon : beacons) {
        if (idx >= (int)childCount || idx >= MAX_STORED_MACS) break;
        
        lv_obj_t* item = lv_obj_get_child(_beaconList, idx);
        if (!item) break;
        
        lv_obj_t* label = lv_obj_get_child(item, 1);
        if (label) {
            String owner = getOwnerTeamName(beacon.macAddress);
            char info[64];
            if (owner.length() > 0) {
                snprintf(info, sizeof(info), "..%s (%d) [%s]",
                         macSuffix(beacon.macAddress), beacon.avgRssi, owner.c_str());
            } else {
                snprintf(info, sizeof(info), "..%s (%d dBm)",
                         macSuffix(beacon.macAddress), beacon.avgRssi);
            }
            lv_label_set_text(label, info);
        }
        idx++;
    }
}

void LVGLBeaconAssignScreen::rebuildBeaconList() {
    if (!_beaconList || !_beaconService) return;
    
    lv_obj_clean(_beaconList);
    
    std::vector<BeaconInfo> beacons = _beaconService->getBeacons();
    _lastBeaconCount = (int)beacons.size();
    
    if (beacons.empty()) {
        lv_obj_t* item = lv_list_add_btn(_beaconList, LV_SYMBOL_BLUETOOTH, "Scanning...");
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    _macCount = 0;
    for (const auto& beacon : beacons) {
        if (_macCount >= MAX_STORED_MACS) break;
        
        strncpy(_macStorage[_macCount], beacon.macAddress.c_str(), 17);
        _macStorage[_macCount][17] = '\0';
        
        String owner = getOwnerTeamName(beacon.macAddress);
        char info[64];
        if (owner.length() > 0) {
            snprintf(info, sizeof(info), "..%s (%d) [%s]",
                     macSuffix(beacon.macAddress), beacon.avgRssi, owner.c_str());
        } else {
            snprintf(info, sizeof(info), "..%s (%d dBm)",
                     macSuffix(beacon.macAddress), beacon.avgRssi);
        }
        
        lv_obj_t* item = lv_list_add_btn(_beaconList, LV_SYMBOL_BLUETOOTH, info);
        lv_obj_set_height(item, 36);
        lv_obj_set_style_pad_ver(item, 4, 0);
        lv_obj_add_event_cb(item, beaconItemEventHandler, LV_EVENT_SHORT_CLICKED, this);
        lv_obj_set_user_data(item, _macStorage[_macCount]);
        
        if (owner.length() > 0) {
            lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        }
        
        _macCount++;
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
    
    lv_obj_t* item = lv_event_get_target(e);
    if (!item) return;
    
    // Walk up to the list button if we clicked on a child (label/icon)
    while (item && lv_obj_get_parent(item) != screen->_beaconList) {
        item = lv_obj_get_parent(item);
    }
    if (!item) return;
    
    const char* mac = (const char*)lv_obj_get_user_data(item);
    if (mac) {
        screen->assignBeaconToTeam(String(mac));
    }
}

void LVGLBeaconAssignScreen::assignBeaconToTeam(const String& beaconUUID) {
    if (!_lapCounter || _teamId == 0) return;
    
    if (_lapCounter->assignBeacon(_teamId, beaconUUID)) {
        Serial.printf("[LVGLBeaconAssign] Beacon %s assigned to team %u\n", beaconUUID.c_str(), _teamId);
        
        if (_navigation) {
            _navigation->goBack();
        }
    }
}

