#ifndef BEACON_SERVICE_H
#define BEACON_SERVICE_H

#include <Arduino.h>
#include "../core/config.h"
#include "../hardware/ble.h"
#include <map>
#include <vector>

/**
 * Beacon Service
 * High-level service for beacon scanning and tracking
 * Handles RSSI averaging, presence tracking, and race-optimized operations
 */

struct BeaconInfo {
    String macAddress;   // MAC address (key)
    String uuid;         // iBeacon UUID or MAC
    uint16_t major;
    uint16_t minor;
    int8_t rssi;        // Current RSSI
    int8_t avgRssi;     // Averaged RSSI (for stability)
    int8_t txPower;
    uint32_t lastSeen;  // millis()
    bool isPresent;     // Currently present (above threshold)
    
    // RSSI averaging (circular buffer)
    static constexpr uint8_t RSSI_HISTORY_SIZE = 5;
    int8_t rssiHistory[RSSI_HISTORY_SIZE];
    uint8_t rssiHistoryIndex;
    uint8_t rssiHistoryCount;
    
    BeaconInfo() : major(0), minor(0), rssi(0), avgRssi(0), txPower(-59), 
                   lastSeen(0), isPresent(false), rssiHistoryIndex(0), rssiHistoryCount(0) {
        for (uint8_t i = 0; i < RSSI_HISTORY_SIZE; i++) {
            rssiHistory[i] = 0;
        }
    }
    
    void updateRSSI(int8_t newRssi) {
        rssi = newRssi;
        
        // Add to history
        rssiHistory[rssiHistoryIndex] = newRssi;
        rssiHistoryIndex = (rssiHistoryIndex + 1) % RSSI_HISTORY_SIZE;
        if (rssiHistoryCount < RSSI_HISTORY_SIZE) {
            rssiHistoryCount++;
        }
        
        // Calculate average
        int32_t sum = 0;
        for (uint8_t i = 0; i < rssiHistoryCount; i++) {
            sum += rssiHistory[i];
        }
        avgRssi = (int8_t)(sum / rssiHistoryCount);
        
        lastSeen = millis();
    }
};

typedef std::function<void(const BeaconInfo&)> BeaconServiceCallback;

/**
 * Beacon Service
 * Manages beacon scanning and tracking with optimizations
 */
class BeaconService {
public:
    BeaconService();
    ~BeaconService();
    
    // Initialization
    bool begin(BLEInterface* ble);
    void end();
    
    // Scanning
    bool startScanning(uint32_t durationMs = 0);
    void stopScanning();
    bool isScanning() const { return _ble && _ble->isScanning(); }
    
    // Callbacks
    void setBeaconCallback(BeaconServiceCallback callback) { _callback = callback; }
    
    // Beacon access
    std::vector<BeaconInfo> getBeacons() const;
    BeaconInfo* getBeacon(const String& macAddress);
    BeaconInfo* getNearestBeacon();
    
    // Configuration
    void setMACFilter(const char* prefix);
    void setRSSIThreshold(int8_t threshold);
    
    // Race mode (optimizations)
    void setRaceMode(bool enabled);
    bool isRaceMode() const { return _raceMode; }
    
    // Cleanup (only when NOT in race mode!)
    void cleanupOldBeacons(uint32_t maxAge = BEACON_TIMEOUT);
    
    // Utils
    static float calculateDistance(int8_t rssi, int8_t txPower = -59);
    
private:
    BLEInterface* _ble;
    BeaconServiceCallback _callback;
    
    std::map<String, BeaconInfo> _beacons;  // MAC address -> BeaconInfo
    bool _raceMode;  // Race mode: no cleanup, all callbacks
    
    // Internal BLE callback handler
    void handleRawBeacon(const BeaconRawData& raw);
};

#endif // BEACON_SERVICE_H

