#ifndef BLE_NIMBLE_H
#define BLE_NIMBLE_H

#include "ble.h"
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>

/**
 * NimBLE Implementation
 * Current implementation using NimBLE library
 */
class BLENimBLE : public BLEInterface {
public:
    BLENimBLE();
    virtual ~BLENimBLE();
    
    bool begin() override;
    void end() override;
    
    bool startScan(uint32_t durationMs = 0) override;
    void stopScan() override;
    bool isScanning() const override { return _scanning; }
    
    void setBeaconCallback(BLEBeaconCallback callback) override { _callback = callback; }
    
    void setMACFilter(const char* prefix) override;
    void setRSSIThreshold(int8_t threshold) override { _rssiThreshold = threshold; }
    
private:
    NimBLEScan* _pBLEScan;
    BLEBeaconCallback _callback;
    char _macFilter[16];
    int8_t _rssiThreshold;
    bool _scanning;
    
    // Internal callback
    class AdvertisedDeviceCallbacks;
    AdvertisedDeviceCallbacks* _callbacks;
    
    bool parseIBeacon(NimBLEAdvertisedDevice* device, BeaconRawData& beacon);
    void disableLogging();
};

#endif // BLE_NIMBLE_H

