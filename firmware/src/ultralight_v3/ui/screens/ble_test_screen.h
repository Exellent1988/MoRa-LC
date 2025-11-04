#ifndef BLE_TEST_SCREEN_H
#define BLE_TEST_SCREEN_H

#include "base_screen.h"
#include "../../services/beacon_service.h"

// Forward declaration
class Navigation;

/**
 * BLE Test Screen
 * Shows detected beacons in real-time
 */
class BLETestScreen : public BaseScreen {
public:
    BLETestScreen(BeaconService* beaconService);
    virtual ~BLETestScreen();
    
    void draw() override;
    void update(uint32_t deltaTime) override;
    bool handleTouch(uint16_t x, uint16_t y) override;
    
    void onEnter() override;
    
    // Set navigation for back button
    void setNavigation(Navigation* nav) { _navigation = nav; }
    
private:
    BeaconService* _beaconService;
    Navigation* _navigation;
    uint32_t _lastUpdate;
    std::vector<BeaconInfo> _beacons;
    uint16_t _scrollOffset;
    
    void drawBeaconList();
    void drawBeaconItem(int index, const BeaconInfo& beacon, int y);
};

#endif // BLE_TEST_SCREEN_H

