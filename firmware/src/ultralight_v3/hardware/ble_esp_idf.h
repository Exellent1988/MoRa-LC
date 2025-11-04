#ifndef BLE_ESP_IDF_H
#define BLE_ESP_IDF_H

#include "ble.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_bt_defs.h"

/**
 * ESP-IDF Native BLE Implementation
 * Direct ESP-IDF API (no Arduino wrapper)
 * Most lightweight and performant option
 */
class BLEESPIDF : public BLEInterface {
public:
    BLEESPIDF();
    virtual ~BLEESPIDF();
    
    bool begin() override;
    void end() override;
    
    bool startScan(uint32_t durationMs = 0) override;
    void stopScan() override;
    bool isScanning() const override { return _scanning; }
    
    void setBeaconCallback(BLEBeaconCallback callback) override { _callback = callback; }
    
    void setMACFilter(const char* prefix) override;
    void setRSSIThreshold(int8_t threshold) override { _rssiThreshold = threshold; }
    
private:
    BLEBeaconCallback _callback;
    char _macFilter[16];
    int8_t _rssiThreshold;
    bool _scanning;
    bool _initialized;
    
    // ESP-IDF scan parameters
    esp_ble_scan_params_t _scanParams;
    
    // Static callback wrapper (ESP-IDF requires static callbacks)
    static void gapEventCallback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    static BLEESPIDF* _instance;  // For static callback access
    
    // Instance callback handler
    void handleGapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    
    bool parseIBeacon(uint8_t* manufacturerData, uint8_t len, BeaconRawData& beacon);
};

#endif // BLE_ESP_IDF_H

