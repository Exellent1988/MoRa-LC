#include "BLEScanner.h"
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <cmath>
#include "esp_log.h"

// Completely disable ESP-IDF logging for NimBLE
// Note: ESP_LOG macros are defined in esp_log.h and called from NimBLE library
// We can't override them here, but we can set log level via build flags
// The actual "I NimBLEScan: Updated advertiser" messages come from ESP-IDF logging

// Disable NimBLE internal logging completely
#ifndef CONFIG_NIMBLE_CPP_LOG_LEVEL
#define CONFIG_NIMBLE_CPP_LOG_LEVEL 0
#endif

// ============================================================
// Internal Callbacks - Defined here before use
// ============================================================

class BLEScanner::AdvertisedDeviceCallbacks : public NimBLEScanCallbacks {
public:
    AdvertisedDeviceCallbacks(BLEScanner* scanner) : scanner(scanner) {}
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice);

private:
    BLEScanner* scanner;
};

// ============================================================
// BLEScanner Implementation
// ============================================================

BLEScanner::BLEScanner() 
    : pBLEScan(nullptr)
    , beaconCallback(nullptr)
    , rssiThreshold(-100)
    , scanning(false)
    , callbacks(nullptr) {
}

BLEScanner::~BLEScanner() {
    stopScan();
    if (callbacks) {
        delete callbacks;
    }
}

bool BLEScanner::begin() {
    Serial.println("[BLE] Initializing...");
    
    // CRITICAL: Disable ESP-IDF logging BEFORE NimBLE init
    // This must be called every time before BLE operations
    esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
    esp_log_level_set("NimBLE", ESP_LOG_NONE);
    esp_log_level_set("BLE", ESP_LOG_NONE);
    esp_log_level_set("BT", ESP_LOG_NONE);
    esp_log_level_set("BTDM", ESP_LOG_NONE);
    esp_log_level_set("BT_HCI", ESP_LOG_NONE);
    
    // Disable NimBLE internal logging to reduce spam
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // Maximum power for better range
    
    pBLEScan = NimBLEDevice::getScan();
    
    if (!pBLEScan) {
        Serial.println("[BLE] ERROR: Failed to create scan");
        return false;
    }
    
    callbacks = new AdvertisedDeviceCallbacks(this);
    pBLEScan->setScanCallbacks(callbacks, false);  // false = wantDuplicates
    
    pBLEScan->setActiveScan(true);   // Active scan für bessere Erkennung
    pBLEScan->setInterval(100);      // ms
    pBLEScan->setWindow(99);         // ms
    pBLEScan->setDuplicateFilter(false);  // Auch Duplicates melden!
    
    Serial.println("[BLE] Initialized successfully");
    return true;
}

void BLEScanner::startScan(uint32_t duration) {
    if (scanning) {
        Serial.println("[BLE] Already scanning");
        return;
    }
    
    // CRITICAL: Disable ESP-IDF logging BEFORE starting scan
    // NimBLE resets log levels when scanning, so we need to set them again
    esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
    esp_log_level_set("NimBLE", ESP_LOG_NONE);
    esp_log_level_set("BLE", ESP_LOG_NONE);
    esp_log_level_set("BT", ESP_LOG_NONE);
    esp_log_level_set("BTDM", ESP_LOG_NONE);
    esp_log_level_set("BT_HCI", ESP_LOG_NONE);
    
    Serial.printf("[BLE] Starting scan (duration: %u ms)\n", duration);
    scanning = true;
    
    // duration = 0 bedeutet kontinuierlich
    pBLEScan->start(duration / 1000, false);
    
    // Set log levels again AFTER start (in case NimBLE reset them)
    delay(10);  // Small delay to let scan initialize
    esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
    esp_log_level_set("NimBLE", ESP_LOG_NONE);
    esp_log_level_set("BLE", ESP_LOG_NONE);
    esp_log_level_set("BT", ESP_LOG_NONE);
    esp_log_level_set("BTDM", ESP_LOG_NONE);
    esp_log_level_set("BT_HCI", ESP_LOG_NONE);
}

void BLEScanner::stopScan() {
    if (!scanning) {
        return;
    }
    
    Serial.println("[BLE] Stopping scan");
    pBLEScan->stop();
    scanning = false;
}

bool BLEScanner::isScanning() {
    return scanning;
}

void BLEScanner::onBeaconDetected(BeaconCallback callback) {
    beaconCallback = callback;
}

std::vector<BeaconData> BLEScanner::getBeacons() {
    std::vector<BeaconData> result;
    for (auto& pair : beacons) {
        result.push_back(pair.second);
    }
    return result;
}

BeaconData* BLEScanner::getBeacon(const String& uuid) {
    auto it = beacons.find(uuid);
    if (it != beacons.end()) {
        return &it->second;
    }
    return nullptr;
}

BeaconData* BLEScanner::getNearestBeacon() {
    BeaconData* nearest = nullptr;
    int8_t maxRSSI = -128;
    
    for (auto& pair : beacons) {
        if (pair.second.rssi > maxRSSI) {
            maxRSSI = pair.second.rssi;
            nearest = &pair.second;
        }
    }
    
    return nearest;
}

void BLEScanner::setUUIDFilter(const String& uuid) {
    uuidFilter = uuid;
    Serial.printf("[BLE] UUID filter set: %s\n", uuid.c_str());
}

void BLEScanner::setRSSIThreshold(int8_t threshold) {
    rssiThreshold = threshold;
    Serial.printf("[BLE] RSSI threshold set: %d dBm\n", threshold);
}

void BLEScanner::clearOldBeacons(uint32_t maxAge) {
    uint32_t now = millis();
    auto it = beacons.begin();
    
    while (it != beacons.end()) {
        if (now - it->second.lastSeen > maxAge) {
            Serial.printf("[BLE] Removing old beacon: %s (age: %u ms)\n", 
                         it->first.c_str(), now - it->second.lastSeen);
            it = beacons.erase(it);
        } else {
            ++it;
        }
    }
}

float BLEScanner::rssiToDistance(int8_t rssi, int8_t txPower) {
    // Log-Distance Path Loss Model
    // d = 10^((txPower - RSSI) / (10 * n))
    // n = 2.5 für Outdoor-Umgebung
    
    if (rssi == 0) {
        return -1.0; // Ungültig
    }
    
    float ratio = (float)(txPower - rssi) / (10.0f * 2.5f);
    return pow(10.0f, ratio);
}

// ============================================================
// Internal Callbacks Implementation
// ============================================================

void BLEScanner::AdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    // CRITICAL: Keep callback FAST - no blocking operations!
    // This callback is called VERY frequently during scanning
    // Any delay here will freeze the UI
    
    // Disable logging (but only once per ~100 calls to save CPU)
    static uint32_t logDisableCounter = 0;
    if (++logDisableCounter % 100 == 0) {
        esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
        esp_log_level_set("NimBLE", ESP_LOG_NONE);
    }
    
    // Quick MAC address check first (fastest filter)
    String macAddress = advertisedDevice->getAddress().toString().c_str();
    
    // MAC-Adresse Filter (nur c3:00:... für Tracking-Beacons) - EARLY RETURN
    if (scanner->uuidFilter.length() > 0) {
        if (!macAddress.startsWith(scanner->uuidFilter)) {
            return;  // Not our beacon, exit immediately
        }
    }
    
    // Quick RSSI check before expensive parsing
    int8_t rssi = advertisedDevice->getRSSI();
    if (rssi < scanner->rssiThreshold) {
        return;  // Too weak, exit immediately
    }
    
    // Now parse beacon data (more expensive operations)
    BeaconData beacon;
    beacon.macAddress = macAddress;
    beacon.rssi = rssi;
    beacon.lastSeen = millis();
    
    // Try to parse as iBeacon (may be expensive, but only for filtered beacons)
    if (!scanner->parseIBeacon(advertisedDevice, beacon)) {
        // Fallback: Use MAC address as UUID for non-iBeacon devices
        beacon.uuid = beacon.macAddress;
        beacon.major = 0;
        beacon.minor = 0;
        beacon.txPower = -59;  // Default TX power
    }
    
    // Update beacon data (Key = MAC-Adresse!)
    String key = beacon.macAddress;
    bool isNew = (scanner->beacons.find(key) == scanner->beacons.end());
    
    // Presence Detection: War Beacon vorher weg?
    if (!isNew) {
        beacon.wasPresent = scanner->beacons[key].wasPresent;
    }
    
    // Store beacon (fast map operation)
    scanner->beacons[key] = beacon;
    scanner->beacons[key].wasPresent = true;
    
    // Only log new beacons (rare, so Serial.print is OK)
    if (isNew) {
        Serial.printf("[BLE] New beacon: MAC=%s, RSSI=%d dBm\n",
                     beacon.macAddress.c_str(), beacon.rssi);
    }
    
    // Callback (may be expensive - but this is needed for lap detection)
    // Only call if callback is set
    if (scanner->beaconCallback) {
        scanner->beaconCallback(scanner->beacons[key]);
    }
}

bool BLEScanner::parseIBeacon(NimBLEAdvertisedDevice* device, BeaconData& beacon) {
    // iBeacon Format:
    // 0x4C 0x00 (Apple Company ID)
    // 0x02 0x15 (iBeacon Type & Length)
    // 16 bytes UUID
    // 2 bytes Major
    // 2 bytes Minor
    // 1 byte TX Power
    
    std::string mfgData = device->getManufacturerData();
    
    if (mfgData.length() < 25) {
        return false;  // Zu kurz für iBeacon
    }
    
    // Check Apple Company ID und iBeacon Type
    if ((uint8_t)mfgData[0] != 0x4C || (uint8_t)mfgData[1] != 0x00 ||
        (uint8_t)mfgData[2] != 0x02 || (uint8_t)mfgData[3] != 0x15) {
        return false;
    }
    
    // UUID extrahieren (16 bytes)
    char uuidStr[37];
    sprintf(uuidStr, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            (uint8_t)mfgData[4], (uint8_t)mfgData[5], (uint8_t)mfgData[6], (uint8_t)mfgData[7],
            (uint8_t)mfgData[8], (uint8_t)mfgData[9], (uint8_t)mfgData[10], (uint8_t)mfgData[11],
            (uint8_t)mfgData[12], (uint8_t)mfgData[13], (uint8_t)mfgData[14], (uint8_t)mfgData[15],
            (uint8_t)mfgData[16], (uint8_t)mfgData[17], (uint8_t)mfgData[18], (uint8_t)mfgData[19]);
    beacon.uuid = String(uuidStr);
    
    // Major (Big Endian)
    beacon.major = ((uint8_t)mfgData[20] << 8) | (uint8_t)mfgData[21];
    
    // Minor (Big Endian)
    beacon.minor = ((uint8_t)mfgData[22] << 8) | (uint8_t)mfgData[23];
    
    // TX Power (signed)
    beacon.txPower = (int8_t)mfgData[24];
    
    // RSSI
    beacon.rssi = device->getRSSI();
    
    // Timestamp
    beacon.lastSeen = millis();
    
    return true;
}




