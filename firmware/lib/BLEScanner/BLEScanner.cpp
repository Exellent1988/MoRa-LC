#include "BLEScanner.h"
#include <cmath>

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
    
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();
    
    if (!pBLEScan) {
        Serial.println("[BLE] ERROR: Failed to create scan");
        return false;
    }
    
    callbacks = new AdvertisedDeviceCallbacks(this);
    pBLEScan->setScanCallbacks(callbacks);
    
    pBLEScan->setActiveScan(true);   // Active scan für bessere Erkennung
    pBLEScan->setInterval(100);      // ms
    pBLEScan->setWindow(99);         // ms
    pBLEScan->setDuplicateFilter(false);  // Auch Duplicates melden!
    
    Serial.println("[BLE] Initialized successfully");
    Serial.println("[BLE] Duplicate filter disabled - will report all beacons");
    return true;
}

void BLEScanner::startScan(uint32_t duration) {
    if (scanning) {
        Serial.println("[BLE] Already scanning");
        return;
    }
    
    Serial.printf("[BLE] Starting scan (duration: %u ms)\n", duration);
    scanning = true;
    
    // duration = 0 bedeutet kontinuierlich
    pBLEScan->start(duration / 1000, false);
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
// Internal Callbacks
// ============================================================

void BLEScanner::AdvertisedDeviceCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
    BeaconData beacon;
    
    // MAC-Adresse immer speichern
    beacon.macAddress = advertisedDevice->getAddress().toString().c_str();
    
    // MAC-Adresse Filter (nur c3:00:... für Tracking-Beacons)
    if (scanner->uuidFilter.length() > 0) {
        if (!beacon.macAddress.startsWith(scanner->uuidFilter)) {
            // Nicht unser Beacon, ignorieren
            return;
        }
    }
    
    // Try to parse as iBeacon first
    if (!scanner->parseIBeacon(advertisedDevice, beacon)) {
        // Fallback: Use MAC address as UUID for non-iBeacon devices
        beacon.uuid = beacon.macAddress;
        beacon.major = 0;
        beacon.minor = 0;
        beacon.txPower = -59;  // Default TX power
        beacon.rssi = advertisedDevice->getRSSI();
        beacon.lastSeen = millis();
        
        Serial.printf("[BLE] Tracking-Beacon: %s (RSSI: %d dBm)\n", 
                     beacon.uuid.c_str(), beacon.rssi);
    } else {
        Serial.printf("[BLE] iBeacon: MAC=%s, UUID=%s (RSSI: %d dBm)\n", 
                     beacon.macAddress.c_str(), beacon.uuid.c_str(), beacon.rssi);
    }
    
    // RSSI Filter (more lenient for testing)
    Serial.printf("[BLE] Checking RSSI: %d vs threshold %d\n", beacon.rssi, scanner->rssiThreshold);
    if (beacon.rssi < scanner->rssiThreshold) {
        Serial.printf("[BLE] ❌ Filtered out: RSSI %d < threshold %d\n", 
                     beacon.rssi, scanner->rssiThreshold);
        return;
    }
    Serial.println("[BLE] ✓ RSSI OK");
    
    // Update beacon data (Key = MAC-Adresse!)
    String key = beacon.macAddress;
    bool isNew = (scanner->beacons.find(key) == scanner->beacons.end());
    
    // Presence Detection: War Beacon vorher weg?
    if (!isNew) {
        beacon.wasPresent = scanner->beacons[key].wasPresent;
    }
    
    scanner->beacons[key] = beacon;
    scanner->beacons[key].wasPresent = true;  // Jetzt ist er da
    
    Serial.printf("[BLE] ✅ Beacon stored! Total beacons: %u\n", scanner->beacons.size());
    
    if (isNew) {
        Serial.printf("[BLE] New beacon: MAC=%s, RSSI=%d dBm, Dist=%.2fm\n",
                     beacon.macAddress.c_str(), beacon.rssi,
                     BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower));
    }
    
    // Callback IMMER aufrufen (auch für Updates!)
    if (scanner->beaconCallback) {
        scanner->beaconCallback(scanner->beacons[key]);
    }
}

bool BLEScanner::parseIBeacon(const NimBLEAdvertisedDevice* device, BeaconData& beacon) {
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



