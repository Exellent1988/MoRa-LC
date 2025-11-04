#ifndef LVGL_BLE_TEST_SCREEN_H
#define LVGL_BLE_TEST_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/beacon_service.h"
#include <lvgl.h>
#include <vector>

// Forward declaration
class LVGLNavigation;

/**
 * BLE Test Screen (LVGL8)
 * Shows detected beacons in real-time with LVGL list widget
 */
class LVGLBLETestScreen : public LVGLBaseScreen {
public:
    LVGLBLETestScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService);
    virtual ~LVGLBLETestScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override;
    
    // Set navigation for back button
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
private:
    BeaconService* _beaconService;
    LVGLNavigation* _navigation;
    lv_obj_t* _list;
    lv_obj_t* _statusLabel;
    uint32_t _lastUpdate;
    std::vector<BeaconInfo> _beacons;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    
    // Helper methods
    void updateBeaconList();
    void addBeaconToList(const BeaconInfo& beacon);
};

#endif // LVGL_BLE_TEST_SCREEN_H

