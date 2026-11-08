#include "BLEScanner.h"
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <NimBLEDevice.h>
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

class BLEScanner::AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
public:
    AdvertisedDeviceCallbacks(BLEScanner* scanner) : scanner(scanner) {}
    void onResult(NimBLEAdvertisedDevice* advertisedDevice);

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
    , mutex(nullptr)
    , sampleQueue(nullptr)
    , dropped(0)
    , listEnabled(true)
    , prefixLen(0)
    , scanning(false)
    , callbacks(nullptr) {
    prefix[0] = 0;
    mutex = xSemaphoreCreateMutex();
    sampleQueue = xQueueCreate(256, sizeof(BeaconSample));
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
    pBLEScan->setAdvertisedDeviceCallbacks(callbacks, false);  // false = wantDuplicates
    
    // Passiv: iBeacon/Tracking-Beacons senden alles im Advertisement. Active Scan
    // erzeugt nur zusaetzlichen Funkverkehr (Scan-Requests/-Responses) und kostet Pakete.
    pBLEScan->setActiveScan(false);
    pBLEScan->setInterval(100);      // ms
    pBLEScan->setWindow(100);        // ms (= Interval: Radio dauerhaft im Scan)
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
    
    // duration = 0 bedeutet kontinuierlich. Nicht-blockierende Variante (kein Callback).
    pBLEScan->start(duration / 1000, nullptr, false);
    
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
    // Scan kann stillschweigend enden (Fehler/Timeout) -> Flag mit Realitaet abgleichen,
    // damit loop() ihn neu startet.
    if (scanning && pBLEScan && !pBLEScan->isScanning()) {
        scanning = false;
    }
    return scanning;
}

bool BLEScanner::popSample(BeaconSample& out) {
    return sampleQueue && xQueueReceive(sampleQueue, &out, 0) == pdTRUE;
}

void BLEScanner::macToString(const uint8_t mac[6], char out[18]) {
    snprintf(out, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
}

void BLEScanner::onBeaconDetected(BeaconCallback callback) {
    beaconCallback = callback;
}

std::vector<BeaconData> BLEScanner::getBeacons() {
    std::vector<BeaconData> result;
    xSemaphoreTake(mutex, portMAX_DELAY);
    for (auto& pair : beacons) {
        result.push_back(pair.second);
    }
    xSemaphoreGive(mutex);
    return result;
}

BeaconData* BLEScanner::getBeacon(const String& uuid) {
    // Hinweis: Zeiger bleibt gueltig bis clearOldBeacons() (laeuft im selben Task wie die UI)
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
    strncpy(prefix, uuid.c_str(), sizeof(prefix) - 1);
    prefix[sizeof(prefix) - 1] = 0;
    for (char* c = prefix; *c; ++c) *c = tolower(*c);
    prefixLen = strlen(prefix);
    Serial.printf("[BLE] UUID filter set: %s\n", uuid.c_str());
}

void BLEScanner::setRSSIThreshold(int8_t threshold) {
    rssiThreshold = threshold;
    Serial.printf("[BLE] RSSI threshold set: %d dBm\n", threshold);
}

void BLEScanner::clearOldBeacons(uint32_t maxAge) {
    uint32_t now = millis();
    xSemaphoreTake(mutex, portMAX_DELAY);
    auto it = beacons.begin();
    
    while (it != beacons.end()) {
        if (now - it->second.lastSeen > maxAge) {
            it = beacons.erase(it);
        } else {
            ++it;
        }
    }
    xSemaphoreGive(mutex);
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
    // Laeuft im NimBLE-Task: schnell bleiben, nichts Blockierendes (kein Serial, keine SD, kein Heap im Hotpath).
    const uint8_t* native = advertisedDevice->getAddress().getNative();

    BeaconSample sample;
    memcpy(sample.mac, native, 6);
    sample.rssi = advertisedDevice->getRSSI();
    sample.ts = millis();

    char macStr[18];
    BLEScanner::macToString(sample.mac, macStr);

    // MAC-Praefix-Filter
    if (scanner->prefixLen > 0 && strncmp(macStr, scanner->prefix, scanner->prefixLen) != 0) {
        return;
    }
    if (sample.rssi < scanner->rssiThreshold) {
        return;
    }

    // 1) Roh-Paket in die Queue (nie blockieren; bei voller Queue zaehlen wir Verluste)
    if (xQueueSend(scanner->sampleQueue, &sample, 0) != pdTRUE) {
        scanner->dropped++;
    }

    // 2) Beacon-Liste fuer die UI (nur wenn gebraucht)
    if (scanner->listEnabled) {
        BeaconData beacon;
        beacon.macAddress = macStr;
        if (!scanner->parseIBeacon(advertisedDevice, beacon)) {
            beacon.uuid = beacon.macAddress;
            beacon.txPower = -59;
            beacon.rssi = sample.rssi;
            beacon.lastSeen = sample.ts;
        }
        if (xSemaphoreTake(scanner->mutex, 0) == pdTRUE) {  // nie warten
            auto it = scanner->beacons.find(beacon.macAddress);
            if (it != scanner->beacons.end()) {
                beacon.wasPresent = it->second.wasPresent;
            }
            beacon.wasPresent = true;
            scanner->beacons[beacon.macAddress] = beacon;
            xSemaphoreGive(scanner->mutex);
        }
    }

    if (scanner->beaconCallback) {
        // Nur fuer Kompatibilitaet; Lap-Erkennung nutzt popSample()
        BeaconData b;
        b.macAddress = macStr;
        b.rssi = sample.rssi;
        b.lastSeen = sample.ts;
        scanner->beaconCallback(b);
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



