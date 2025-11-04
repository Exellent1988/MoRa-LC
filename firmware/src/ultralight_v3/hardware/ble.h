#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include "../core/config.h"

/**
 * BLE Abstraction Layer
 * Allows switching between different BLE libraries (NimBLE, ESP32 BLE, etc.)
 */

struct BeaconRawData {
    char macAddress[18];  // Use char array instead of String for performance
    char uuid[37];
    uint16_t major;
    uint16_t minor;
    int8_t rssi;
    int8_t txPower;
    uint32_t timestamp;  // millis()
    
    BeaconRawData() : major(0), minor(0), rssi(0), txPower(-59), timestamp(0) {
        macAddress[0] = '\0';
        uuid[0] = '\0';
    }
};

typedef std::function<void(const BeaconRawData&)> BLEBeaconCallback;

/**
 * BLE Interface - Abstract base class
 */
class BLEInterface {
public:
    virtual ~BLEInterface() {}
    
    // Initialization
    virtual bool begin() = 0;
    virtual void end() = 0;
    
    // Scanning
    virtual bool startScan(uint32_t durationMs = 0) = 0;  // 0 = continuous
    virtual void stopScan() = 0;
    virtual bool isScanning() const = 0;
    
    // Callbacks
    virtual void setBeaconCallback(BLEBeaconCallback callback) = 0;
    
    // Configuration
    virtual void setMACFilter(const char* prefix) = 0;  // e.g., "c3:00:"
    virtual void setRSSIThreshold(int8_t threshold) = 0;
    
    // Utils
    static float rssiToDistance(int8_t rssi, int8_t txPower = -59);
};

#endif // BLE_H

