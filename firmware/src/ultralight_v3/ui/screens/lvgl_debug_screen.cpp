#include "lvgl_debug_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

static const char* macSuffix(const String& mac) {
    int len = mac.length();
    return (len > 5) ? mac.c_str() + len - 5 : mac.c_str();
}

LVGLDebugScreen::LVGLDebugScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _beaconService(beaconService)
    , _lapCounter(lapCounter)
    , _list(nullptr)
    , _lastUpdate(0) {
}

LVGLDebugScreen::~LVGLDebugScreen() {
}

void LVGLDebugScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) return;
    
    _screen = _lvglDisplay->getScreen();
    lv_obj_clean(_screen);
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    createHeader("Debug", true, backBtnEventHandler, this);
    
    int listY = HEADER_HEIGHT + 2;
    int listH = SCREEN_HEIGHT - listY;
    _list = createList(0, listY, SCREEN_WIDTH, listH);
    
    _lastUpdate = 0;
    refreshList();
}

void LVGLDebugScreen::onExit() {
}

void LVGLDebugScreen::update() {
    uint32_t now = millis();
    if (now - _lastUpdate > 500) {
        refreshList();
        _lastUpdate = now;
    }
}

void LVGLDebugScreen::refreshList() {
    if (!_list || !_beaconService || !_lapCounter) return;
    
    lv_obj_clean(_list);
    
    int8_t rssiNear = _lapCounter->getRssiNear();
    int8_t rssiFar = _lapCounter->getRssiFar();
    
    // Show each team with beacon status
    auto teams = _lapCounter->getAllTeams();
    for (auto* team : teams) {
        char info[96];
        
        if (team->beaconUUID.length() == 0) {
            snprintf(info, sizeof(info), "%s: no beacon", team->teamName.c_str());
            lv_obj_t* item = lv_list_add_btn(_list, NULL, info);
            lv_obj_set_height(item, 26);
            lv_obj_set_style_pad_ver(item, 2, 0);
            lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
            continue;
        }
        
        BeaconInfo* beacon = _beaconService->getBeacon(team->beaconUUID);
        if (!beacon) {
            snprintf(info, sizeof(info), "%s [%s]: not seen",
                     team->teamName.c_str(), macSuffix(team->beaconUUID));
            lv_obj_t* item = lv_list_add_btn(_list, NULL, info);
            lv_obj_set_height(item, 26);
            lv_obj_set_style_pad_ver(item, 2, 0);
            lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
            continue;
        }
        
        // Determine near/far status
        const char* status;
        uint16_t bgColor;
        if (beacon->avgRssi >= rssiNear) {
            status = "NEAR";
            bgColor = Colors::SECONDARY;  // green
        } else if (beacon->avgRssi <= rssiFar) {
            status = "FAR";
            bgColor = Colors::DANGER;     // red
        } else {
            status = "MID";
            bgColor = Colors::WARNING;    // orange
        }
        
        snprintf(info, sizeof(info), "%s [%s] %d/%d %s L%u",
                 team->teamName.c_str(), macSuffix(team->beaconUUID),
                 beacon->rssi, beacon->avgRssi, status, team->lapCount);
        
        lv_obj_t* item = lv_list_add_btn(_list, NULL, info);
        lv_obj_set_height(item, 26);
        lv_obj_set_style_pad_ver(item, 2, 0);
        lv_obj_set_style_bg_color(item, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
        
        // Color-code the entire item background
        lv_color_t c;
        c.full = bgColor;
        lv_obj_set_style_bg_color(item, c, 0);
        lv_obj_set_style_bg_opa(item, LV_OPA_COVER, 0);
    }
    
    // Also show unassigned beacons
    auto beacons = _beaconService->getBeacons();
    for (const auto& b : beacons) {
        bool assigned = false;
        for (auto* t : teams) {
            if (t->beaconUUID == b.macAddress) { assigned = true; break; }
        }
        if (assigned) continue;
        
        char info[64];
        snprintf(info, sizeof(info), "-- [%s] %d dBm", macSuffix(b.macAddress), b.avgRssi);
        lv_obj_t* item = lv_list_add_btn(_list, NULL, info);
        lv_obj_set_height(item, 22);
        lv_obj_set_style_pad_ver(item, 1, 0);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
    }
    
    // Show thresholds at bottom
    char threshInfo[48];
    snprintf(threshInfo, sizeof(threshInfo), "Near>=%d  Far<=%d", rssiNear, rssiFar);
    lv_obj_t* threshItem = lv_list_add_btn(_list, NULL, threshInfo);
    lv_obj_set_height(threshItem, 20);
    lv_obj_set_style_pad_ver(threshItem, 1, 0);
}

void LVGLDebugScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLDebugScreen* screen = (LVGLDebugScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        screen->_navigation->goBack();
    }
}
