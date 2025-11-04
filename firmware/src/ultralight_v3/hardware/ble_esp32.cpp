#include "ble_esp32.h"
#include <cmath>
#include <string>
#include <cstring>

// Internal callback class
class BLEESP32::AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
public:
    AdvertisedDeviceCallbacks(BLEESP32* ble) : _ble(ble) {}
    
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        // Keep callback FAST - minimal operations
        BeaconRawData beacon;
        
        // Get MAC address (ESP32 BLE uses by-value parameter)
        BLEAddress addr = advertisedDevice.getAddress();
        const char* macCStr = addr.toString().c_str();
        strncpy(beacon.macAddress, macCStr, sizeof(beacon.macAddress) - 1);
        beacon.macAddress[sizeof(beacon.macAddress) - 1] = '\0';
        
        // MAC filter (early exit for performance)
        if (_ble->_macFilter[0] != '\0') {
            if (strncmp(beacon.macAddress, _ble->_macFilter, strlen(_ble->_macFilter)) != 0) {
                return;
            }
        }
        
        // Parse iBeacon (pass by reference to avoid copy)
        if (!_ble->parseIBeacon(&advertisedDevice, beacon)) {
            // Fallback: Use MAC as UUID
            strncpy(beacon.uuid, beacon.macAddress, sizeof(beacon.uuid) - 1);
            beacon.uuid[sizeof(beacon.uuid) - 1] = '\0';
            beacon.major = 0;
            beacon.minor = 0;
            beacon.txPower = -59;
            beacon.rssi = advertisedDevice.getRSSI();
            beacon.timestamp = millis();
        }
        
        // RSSI filter
        if (beacon.rssi < _ble->_rssiThreshold) {
            return;
        }
        
        // Call callback (always, even for updates - critical for race tracking!)
        if (_ble->_callback) {
            _ble->_callback(beacon);
        }
    }
    
private:
    BLEESP32* _ble;
};

BLEESP32::BLEESP32()
    : _pBLEScan(nullptr)
    , _callback(nullptr)
    , _rssiThreshold(BLE_RSSI_THRESHOLD)
    , _scanning(false)
    , _callbacks(nullptr) {
    _macFilter[0] = '\0';
}

BLEESP32::~BLEESP32() {
    end();
    if (_callbacks) {
        delete _callbacks;
    }
}

bool BLEESP32::begin() {
    Serial.println("[BLE] Initializing ESP32 BLE Arduino...");
    
    // Initialize BLE device
    BLEDevice::init("");
    
    // Set power to maximum
    BLEDevice::setPower(ESP_PWR_LVL_P9);
    
    // Create scan
    _pBLEScan = BLEDevice::getScan();
    if (!_pBLEScan) {
        Serial.println("[BLE] ERROR: Failed to create scan");
        return false;
    }
    
    // Set callback
    _callbacks = new AdvertisedDeviceCallbacks(this);
    _pBLEScan->setAdvertisedDeviceCallbacks(_callbacks);
    
    // Configure scan parameters
    _pBLEScan->setActiveScan(true);  // Active scan
    _pBLEScan->setInterval(BLE_SCAN_INTERVAL);
    _pBLEScan->setWindow(BLE_SCAN_WINDOW);
    // Note: ESP32 BLE Arduino Library internally filters duplicates
    // We need to restart scan periodically to get continuous updates
    
    // Set MAC filter if configured
    if (_macFilter[0] != '\0') {
        setMACFilter(_macFilter);
    }
    
    Serial.println("[BLE] ESP32 BLE initialized successfully");
    return true;
}

void BLEESP32::end() {
    stopScan();
    if (_pBLEScan) {
        BLEDevice::deinit(true);  // true = clear all data
        _pBLEScan = nullptr;
    }
}

bool BLEESP32::startScan(uint32_t durationMs) {
    if (_scanning) {
        return true;
    }
    
    Serial.printf("[BLE] Starting scan (duration: %u ms)\n", durationMs);
    _scanning = true;
    
    // ESP32 BLE Arduino: For continuous scanning, we need to restart the scan periodically
    // because the library filters duplicates internally
    if (durationMs == 0) {
        // Continuous mode: Start with a short duration, then restart
        _pBLEScan->start(5, false);  // 5 seconds, false = don't delete results
    } else {
        uint32_t scanDuration = durationMs / 1000;
        _pBLEScan->start(scanDuration, false);
    }
    
    return true;
}

void BLEESP32::stopScan() {
    if (!_scanning) {
        return;
    }
    
    // Serial.println("[BLE] Stopping scan");  // Commented out to reduce spam
    if (_pBLEScan) {
        _pBLEScan->stop();
    }
    _scanning = false;
    delay(50);  // Small delay to let scan fully stop
}

void BLEESP32::setMACFilter(const char* prefix) {
    if (prefix) {
        strncpy(_macFilter, prefix, sizeof(_macFilter) - 1);
        _macFilter[sizeof(_macFilter) - 1] = '\0';
        Serial.printf("[BLE] MAC filter set: %s\n", _macFilter);
    } else {
        _macFilter[0] = '\0';
    }
}

bool BLEESP32::parseIBeacon(BLEAdvertisedDevice* device, BeaconRawData& beacon) {
    std::string mfgData = device->getManufacturerData();
    
    if (mfgData.length() < 25) {
        return false;
    }
    
    // Check Apple Company ID and iBeacon type
    if ((uint8_t)mfgData[0] != 0x4C || (uint8_t)mfgData[1] != 0x00 ||
        (uint8_t)mfgData[2] != 0x02 || (uint8_t)mfgData[3] != 0x15) {
        return false;
    }
    
    // Extract UUID (16 bytes)
    char uuidStr[37];
    sprintf(uuidStr, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            (uint8_t)mfgData[4], (uint8_t)mfgData[5], (uint8_t)mfgData[6], (uint8_t)mfgData[7],
            (uint8_t)mfgData[8], (uint8_t)mfgData[9], (uint8_t)mfgData[10], (uint8_t)mfgData[11],
            (uint8_t)mfgData[12], (uint8_t)mfgData[13], (uint8_t)mfgData[14], (uint8_t)mfgData[15],
            (uint8_t)mfgData[16], (uint8_t)mfgData[17], (uint8_t)mfgData[18], (uint8_t)mfgData[19]);
    strncpy(beacon.uuid, uuidStr, sizeof(beacon.uuid) - 1);
    beacon.uuid[sizeof(beacon.uuid) - 1] = '\0';
    
    // Major (Big Endian)
    beacon.major = ((uint8_t)mfgData[20] << 8) | (uint8_t)mfgData[21];
    
    // Minor (Big Endian)
    beacon.minor = ((uint8_t)mfgData[22] << 8) | (uint8_t)mfgData[23];
    
    // TX Power (signed)
    beacon.txPower = (int8_t)mfgData[24];
    
    // RSSI and timestamp
    beacon.rssi = device->getRSSI();
    beacon.timestamp = millis();
    
    return true;
}

