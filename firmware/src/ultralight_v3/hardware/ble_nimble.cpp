#include "ble_nimble.h"
#include "esp_log.h"
#include <cmath>
#include <string>
#include <cstring>

// Internal callback class
class BLENimBLE::AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
public:
    AdvertisedDeviceCallbacks(BLENimBLE* ble) : _ble(ble) {}
    
    void onResult(NimBLEAdvertisedDevice* device) override {
        // CRITICAL: Keep callback FAST - minimal operations
        // Disable logging (NimBLE might re-enable it)
        _ble->disableLogging();
        
        BeaconRawData beacon;
        
        // Get MAC address (use char array for performance)
        // NimBLE toString() returns std::string
        std::string macStr = device->getAddress().toString();
        const char* macCStr = macStr.c_str();
        strncpy(beacon.macAddress, macCStr, sizeof(beacon.macAddress) - 1);
        beacon.macAddress[sizeof(beacon.macAddress) - 1] = '\0';
        
        // MAC filter (early exit for performance)
        if (_ble->_macFilter[0] != '\0') {
            if (strncmp(beacon.macAddress, _ble->_macFilter, strlen(_ble->_macFilter)) != 0) {
                return;
            }
        }
        
        // Parse iBeacon
        if (!_ble->parseIBeacon(device, beacon)) {
            // Fallback: Use MAC as UUID
            strncpy(beacon.uuid, beacon.macAddress, sizeof(beacon.uuid) - 1);
            beacon.uuid[sizeof(beacon.uuid) - 1] = '\0';
            beacon.major = 0;
            beacon.minor = 0;
            beacon.txPower = -59;
            beacon.rssi = device->getRSSI();
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
    BLENimBLE* _ble;
};

BLENimBLE::BLENimBLE()
    : _pBLEScan(nullptr)
    , _callback(nullptr)
    , _rssiThreshold(BLE_RSSI_THRESHOLD)
    , _scanning(false)
    , _callbacks(nullptr) {
    _macFilter[0] = '\0';
}

BLENimBLE::~BLENimBLE() {
    end();
    if (_callbacks) {
        delete _callbacks;
    }
}

void BLENimBLE::disableLogging() {
    // Disable ESP-IDF logging (called frequently in callbacks)
    esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_level_set("NimBLEScan", ESP_LOG_NONE);
    esp_log_level_set("NimBLE", ESP_LOG_NONE);
    esp_log_level_set("BLE", ESP_LOG_NONE);
    esp_log_level_set("BT", ESP_LOG_NONE);
    esp_log_level_set("BTDM", ESP_LOG_NONE);
    esp_log_level_set("BT_HCI", ESP_LOG_NONE);
}

bool BLENimBLE::begin() {
    Serial.println("[BLE] Initializing NimBLE...");
    
    // Disable logging BEFORE init
    disableLogging();
    
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // Maximum power
    
    _pBLEScan = NimBLEDevice::getScan();
    if (!_pBLEScan) {
        Serial.println("[BLE] ERROR: Failed to create scan");
        return false;
    }
    
    _callbacks = new AdvertisedDeviceCallbacks(this);
    _pBLEScan->setAdvertisedDeviceCallbacks(_callbacks, false);  // false = wantDuplicates
    
    _pBLEScan->setActiveScan(true);      // Active scan
    _pBLEScan->setInterval(BLE_SCAN_INTERVAL);
    _pBLEScan->setWindow(BLE_SCAN_WINDOW);
    _pBLEScan->setDuplicateFilter(false);  // Report duplicates (needed for updates)
    
    // Set MAC filter if configured
    if (_macFilter[0] != '\0') {
        setMACFilter(_macFilter);
    }
    
    Serial.println("[BLE] NimBLE initialized successfully");
    return true;
}

void BLENimBLE::end() {
    stopScan();
    if (_pBLEScan) {
        NimBLEDevice::deinit(false);
        _pBLEScan = nullptr;
    }
}

bool BLENimBLE::startScan(uint32_t durationMs) {
    if (_scanning) {
        return true;
    }
    
    // Disable logging before scan
    disableLogging();
    
    Serial.printf("[BLE] Starting scan (duration: %u ms)\n", durationMs);
    _scanning = true;
    
    uint32_t scanDuration = (durationMs == 0) ? 0 : (durationMs / 1000);
    _pBLEScan->start(scanDuration, false);  // false = non-blocking, don't restart
    
    // Disable logging again after start
    delay(10);
    disableLogging();
    
    return true;
}

void BLENimBLE::stopScan() {
    if (!_scanning) {
        return;
    }
    
    Serial.println("[BLE] Stopping scan");
    if (_pBLEScan) {
        _pBLEScan->stop();
    }
    _scanning = false;
}

void BLENimBLE::setMACFilter(const char* prefix) {
    if (prefix) {
        strncpy(_macFilter, prefix, sizeof(_macFilter) - 1);
        _macFilter[sizeof(_macFilter) - 1] = '\0';
        Serial.printf("[BLE] MAC filter set: %s\n", _macFilter);
    } else {
        _macFilter[0] = '\0';
    }
}

bool BLENimBLE::parseIBeacon(NimBLEAdvertisedDevice* device, BeaconRawData& beacon) {
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

float BLEInterface::rssiToDistance(int8_t rssi, int8_t txPower) {
    if (rssi == 0) {
        return -1.0;
    }
    
    float ratio = (float)(txPower - rssi) / (10.0f * 2.5f);
    return pow(10.0f, ratio);
}

