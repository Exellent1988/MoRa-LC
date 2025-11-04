#ifndef BLE_ESP32_H
#define BLE_ESP32_H

#include "ble.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

/**
 * ESP32 BLE Arduino Implementation
 * Alternative to NimBLE using the standard ESP32 BLE library
 */
class BLEESP32 : public BLEInterface {
public:
    BLEESP32();
    virtual ~BLEESP32();
    
    bool begin() override;
    void end() override;
    
    bool startScan(uint32_t durationMs = 0) override;
    void stopScan() override;
    bool isScanning() const override { return _scanning; }
    
    void setBeaconCallback(BLEBeaconCallback callback) override { _callback = callback; }
    
    void setMACFilter(const char* prefix) override;
    void setRSSIThreshold(int8_t threshold) override { _rssiThreshold = threshold; }
    
private:
    BLEScan* _pBLEScan;
    BLEBeaconCallback _callback;
    char _macFilter[16];
    int8_t _rssiThreshold;
    bool _scanning;
    
    // Internal callback
    class AdvertisedDeviceCallbacks;
    AdvertisedDeviceCallbacks* _callbacks;
    
    bool parseIBeacon(BLEAdvertisedDevice* device, BeaconRawData& beacon);
};

#endif // BLE_ESP32_H

