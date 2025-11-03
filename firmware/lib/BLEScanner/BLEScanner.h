#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <vector>
#include <map>

// Forward declarations to avoid circular includes
class NimBLEScan;
class NimBLEScanCallbacks;
class NimBLEAdvertisedDevice;

/**
 * BLE Scanner für iBeacon Erkennung
 * 
 * Scannt nach BLE iBeacons (Checkpoints und andere Beacons)
 * Beide Varianten (FullBlown & UltraLight) nutzen diese Library
 */

struct BeaconData {
    String uuid;        // iBeacon UUID oder MAC-Adresse
    String macAddress;  // Immer die MAC-Adresse (für Zuordnung)
    uint16_t major;
    uint16_t minor;
    int8_t rssi;
    int8_t txPower;
    uint32_t lastSeen;  // millis()
    bool wasPresent;    // Für Presence Detection
    
    BeaconData() : major(0), minor(0), rssi(0), txPower(-59), lastSeen(0), wasPresent(false) {}
};

typedef std::function<void(const BeaconData&)> BeaconCallback;

class BLEScanner {
public:
    BLEScanner();
    ~BLEScanner();
    
    // Initialisierung
    bool begin();
    
    // Scanning
    void startScan(uint32_t duration = 0); // 0 = kontinuierlich
    void stopScan();
    bool isScanning();
    
    // Callback für neue Beacons
    void onBeaconDetected(BeaconCallback callback);
    
    // Beacon-Daten abrufen
    std::vector<BeaconData> getBeacons();
    BeaconData* getBeacon(const String& uuid);
    BeaconData* getNearestBeacon();  // Beacon mit stärkstem RSSI
    
    // Filtering
    void setUUIDFilter(const String& uuid);  // Nur diesen UUID scannen
    void setRSSIThreshold(int8_t threshold); // Nur Beacons >= threshold
    
    // Cleanup
    void clearOldBeacons(uint32_t maxAge = 5000);  // Entferne Beacons älter als maxAge ms
    
    // Utils
    static float rssiToDistance(int8_t rssi, int8_t txPower = -59);
    
private:
    NimBLEScan* pBLEScan;
    BeaconCallback beaconCallback;
    
    std::map<String, BeaconData> beacons;  // UUID -> BeaconData
    
    String uuidFilter;
    int8_t rssiThreshold;
    bool scanning;
    
    // Internal callback - forward declare to avoid include issues
    class AdvertisedDeviceCallbacks;
    
    AdvertisedDeviceCallbacks* callbacks;
    
    // Helper
    bool parseIBeacon(NimBLEAdvertisedDevice* device, BeaconData& beacon);
};

#endif // BLE_SCANNER_H



