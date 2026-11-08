#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <vector>
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// Forward declarations to avoid circular includes
class NimBLEScan;
class NimBLEAdvertisedDeviceCallbacks;
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

// Roh-Paket aus dem BLE-Task (keine Heap-Objekte -> sicher fuer Queue)
struct BeaconSample {
    uint8_t  mac[6];   // NimBLE-Reihenfolge (LSB zuerst)
    int8_t   rssi;
    uint32_t ts;       // millis() beim Empfang
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
    
    // Callback für neue Beacons (Achtung: laeuft im BLE-Task, nur fuer leichte Aufgaben!)
    void onBeaconDetected(BeaconCallback callback);

    // Entkoppelter Pfad: Pakete der gefilterten Beacons landen in einer Queue.
    // Aus loop() mit popSample() abholen.
    bool popSample(BeaconSample& out);
    uint32_t droppedSamples() const { return dropped; }
    static void macToString(const uint8_t mac[6], char out[18]);  // "c3:00:..." (lowercase)

    // Beacon-Liste (fuer UI) nur pflegen wenn noetig - spart waehrend des Rennens Heap/CPU
    void setListEnabled(bool enabled) { listEnabled = enabled; }
    
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
    SemaphoreHandle_t mutex;
    QueueHandle_t sampleQueue;
    volatile uint32_t dropped;
    volatile bool listEnabled;
    char prefix[18];  // MAC-Praefix-Filter (lowercase), leer = alle
    size_t prefixLen;
    
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



