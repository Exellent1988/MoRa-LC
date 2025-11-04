#include "ble_esp_idf.h"
#include <cstring>
#include <cmath>

// Static instance for callback access
BLEESPIDF* BLEESPIDF::_instance = nullptr;

BLEESPIDF::BLEESPIDF()
    : _callback(nullptr)
    , _rssiThreshold(BLE_RSSI_THRESHOLD)
    , _scanning(false)
    , _initialized(false) {
    _macFilter[0] = '\0';
    memset(&_scanParams, 0, sizeof(_scanParams));
}

BLEESPIDF::~BLEESPIDF() {
    end();
}

bool BLEESPIDF::begin() {
    Serial.println("[BLE] Initializing ESP-IDF Native BLE...");
    
    // Set instance for static callback
    _instance = this;
    
    // Initialize BT controller
    // First, try to deinit if already initialized (from previous attempt)
    esp_bt_controller_status_t status = esp_bt_controller_get_status();
    if (status != ESP_BT_CONTROLLER_STATUS_IDLE) {
        Serial.printf("[BLE] BT controller already active (status: %d), cleaning up...\n", status);
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        delay(100);  // Give it time to clean up
    }
    
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        Serial.printf("[BLE] ERROR: Failed to init BT controller: %d\n", ret);
        return false;
    }
    
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        Serial.printf("[BLE] ERROR: Failed to enable BT controller: %d\n", ret);
        return false;
    }
    
    // Initialize BLE stack
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        Serial.printf("[BLE] ERROR: Failed to init bluedroid: %d\n", ret);
        return false;
    }
    
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        Serial.printf("[BLE] ERROR: Failed to enable bluedroid: %d\n", ret);
        return false;
    }
    
    // Register GAP callback
    ret = esp_ble_gap_register_callback(gapEventCallback);
    if (ret != ESP_OK) {
        Serial.printf("[BLE] ERROR: Failed to register GAP callback: %d\n", ret);
        return false;
    }
    
    // Set scan parameters
    _scanParams.scan_type = BLE_SCAN_TYPE_ACTIVE;
    _scanParams.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    _scanParams.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
    _scanParams.scan_interval = BLE_SCAN_INTERVAL;
    _scanParams.scan_window = BLE_SCAN_WINDOW;
    _scanParams.scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE;  // Enable duplicates!
    
    // Set MAC filter if configured
    if (_macFilter[0] != '\0') {
        setMACFilter(_macFilter);
    }
    
    _initialized = true;
    Serial.println("[BLE] ESP-IDF Native BLE initialized successfully");
    return true;
}

void BLEESPIDF::end() {
    stopScan();
    
    if (_initialized) {
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        _initialized = false;
    }
    
    _instance = nullptr;
}

bool BLEESPIDF::startScan(uint32_t durationMs) {
    if (!_initialized || _scanning) {
        return false;
    }
    
    Serial.printf("[BLE] Starting scan (duration: %u ms)\n", durationMs);
    
    // Convert duration to seconds (0 = continuous)
    uint32_t duration = (durationMs == 0) ? 0 : (durationMs / 1000);
    
    esp_err_t ret = esp_ble_gap_start_scanning(duration);
    if (ret != ESP_OK) {
        Serial.printf("[BLE] ERROR: Failed to start scan: %d\n", ret);
        return false;
    }
    
    _scanning = true;
    return true;
}

void BLEESPIDF::stopScan() {
    if (!_scanning) {
        return;
    }
    
    esp_ble_gap_stop_scanning();
    _scanning = false;
}

void BLEESPIDF::setMACFilter(const char* prefix) {
    if (prefix) {
        strncpy(_macFilter, prefix, sizeof(_macFilter) - 1);
        _macFilter[sizeof(_macFilter) - 1] = '\0';
        Serial.printf("[BLE] MAC filter set: %s\n", _macFilter);
    } else {
        _macFilter[0] = '\0';
    }
}

// Static callback wrapper
void BLEESPIDF::gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (_instance) {
        _instance->handleGapEvent(event, param);
    }
}

// Instance callback handler
void BLEESPIDF::handleGapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_SCAN_RESULT_EVT: {
            if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
                // Device found
                esp_ble_gap_cb_param_t* scan_result = param;
                esp_ble_gap_cb_param_t::ble_scan_result_evt_param* ble_scan_result = &scan_result->scan_rst;
                
                BeaconRawData beacon;
                
                // Get MAC address
                char macStr[18];
                sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x",
                       ble_scan_result->bda[0], ble_scan_result->bda[1],
                       ble_scan_result->bda[2], ble_scan_result->bda[3],
                       ble_scan_result->bda[4], ble_scan_result->bda[5]);
                strncpy(beacon.macAddress, macStr, sizeof(beacon.macAddress) - 1);
                beacon.macAddress[sizeof(beacon.macAddress) - 1] = '\0';
                
                // MAC filter
                if (_macFilter[0] != '\0') {
                    if (strncmp(beacon.macAddress, _macFilter, strlen(_macFilter)) != 0) {
                        return;
                    }
                }
                
                // Get RSSI
                beacon.rssi = ble_scan_result->rssi;
                beacon.timestamp = millis();
                
                // RSSI filter
                if (beacon.rssi < _rssiThreshold) {
                    return;
                }
                
                // Parse iBeacon from advertising data
                bool iBeaconFound = false;
                if (ble_scan_result->adv_data_len > 0) {
                    uint8_t* advData = ble_scan_result->ble_adv;
                    uint8_t advLen = ble_scan_result->adv_data_len;
                    
                    // Parse advertising data to find manufacturer data
                    uint8_t pos = 0;
                    while (pos < advLen - 1) {
                        uint8_t fieldLen = advData[pos++];
                        if (fieldLen == 0 || pos + fieldLen > advLen) break;
                        
                        uint8_t fieldType = advData[pos++];
                        
                        if (fieldType == ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE) {
                            // Manufacturer data (iBeacon)
                            uint8_t mfgDataLen = fieldLen - 1;
                            if (mfgDataLen >= 25) {
                                iBeaconFound = parseIBeacon(&advData[pos], mfgDataLen, beacon);
                            }
                            break;
                        }
                        
                        pos += fieldLen - 1;
                    }
                }
                
                if (!iBeaconFound) {
                    // Fallback: Use MAC as UUID
                    strncpy(beacon.uuid, beacon.macAddress, sizeof(beacon.uuid) - 1);
                    beacon.uuid[sizeof(beacon.uuid) - 1] = '\0';
                    beacon.major = 0;
                    beacon.minor = 0;
                    beacon.txPower = -59;
                }
                
                // Call callback
                if (_callback) {
                    _callback(beacon);
                }
            } else if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT) {
                // Scan complete
                _scanning = false;
            }
            break;
        }
        
        default:
            break;
    }
}

bool BLEESPIDF::parseIBeacon(uint8_t* manufacturerData, uint8_t len, BeaconRawData& beacon) {
    if (len < 25) {
        return false;
    }
    
    // Check Apple Company ID (0x004C) and iBeacon type (0x02 0x15)
    if (manufacturerData[0] != 0x4C || manufacturerData[1] != 0x00 ||
        manufacturerData[2] != 0x02 || manufacturerData[3] != 0x15) {
        return false;
    }
    
    // Extract UUID (16 bytes, starting at index 4)
    char uuidStr[37];
    sprintf(uuidStr, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            manufacturerData[4], manufacturerData[5], manufacturerData[6], manufacturerData[7],
            manufacturerData[8], manufacturerData[9], manufacturerData[10], manufacturerData[11],
            manufacturerData[12], manufacturerData[13], manufacturerData[14], manufacturerData[15],
            manufacturerData[16], manufacturerData[17], manufacturerData[18], manufacturerData[19]);
    strncpy(beacon.uuid, uuidStr, sizeof(beacon.uuid) - 1);
    beacon.uuid[sizeof(beacon.uuid) - 1] = '\0';
    
    // Major (Big Endian, bytes 20-21)
    beacon.major = (manufacturerData[20] << 8) | manufacturerData[21];
    
    // Minor (Big Endian, bytes 22-23)
    beacon.minor = (manufacturerData[22] << 8) | manufacturerData[23];
    
    // TX Power (signed, byte 24)
    beacon.txPower = (int8_t)manufacturerData[24];
    
    return true;
}

