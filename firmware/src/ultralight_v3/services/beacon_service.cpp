#include "beacon_service.h"
#include <cmath>
#include <cstring>

BeaconService::BeaconService()
    : _ble(nullptr)
    , _callback(nullptr)
    , _raceMode(false)
    , _rssiThreshold(BLE_RSSI_THRESHOLD) {
}

BeaconService::~BeaconService() {
    end();
}

bool BeaconService::begin(BLEInterface* ble) {
    if (!ble) {
        Serial.println("[BeaconService] ERROR: BLE interface is null");
        return false;
    }
    
    _ble = ble;
    
    // Set up BLE callback - use lambda to capture 'this'
    _ble->setBeaconCallback([this](const BeaconRawData& raw) {
        this->handleRawBeacon(raw);
    });
      _ble->setRSSIThreshold(_rssiThreshold);
    
    Serial.println("[BeaconService] Initialized");
    return true;
}

void BeaconService::end() {
    stopScanning();
    _beacons.clear();
    _ble = nullptr;
}

bool BeaconService::startScanning(uint32_t durationMs) {
    if (!_ble) {
        return false;
    }
    
    return _ble->startScan(durationMs);
}

void BeaconService::stopScanning() {
    if (_ble) {
        _ble->stopScan();
    }
}

void BeaconService::setMACFilter(const char* prefix) {
    if (_ble) {
        _ble->setMACFilter(prefix);
    }
}

void BeaconService::setRSSIThreshold(int8_t threshold) {
    if (_ble) {
        _ble->setRSSIThreshold(threshold);
    }
    _rssiThreshold = threshold;
}

void BeaconService::setRaceMode(bool enabled) {
    _raceMode = enabled;
    Serial.printf("[BeaconService] Race mode: %s\n", enabled ? "ON" : "OFF");
    
    // CRITICAL: In race mode, NEVER cleanup beacons!
    // This ensures beacons remain tracked even with temporary signal loss
}

void BeaconService::cleanupOldBeacons(uint32_t maxAge) {
    // CRITICAL: Never cleanup during race!
    if (_raceMode) {
        return;
    }
    
    uint32_t now = millis();
    auto it = _beacons.begin();
    
    while (it != _beacons.end()) {
        if (now - it->second.lastSeen > maxAge) {
            Serial.printf("[BeaconService] Removing old beacon: %s (age: %lu ms)\n",
                         it->first.c_str(), now - it->second.lastSeen);
            it = _beacons.erase(it);
        } else {
            ++it;
        }
    }
}

void BeaconService::handleRawBeacon(const BeaconRawData& raw) {
    String macKey = String(raw.macAddress);
    
    // Find or create beacon
    auto it = _beacons.find(macKey);
    bool isNew = (it == _beacons.end());
    
    BeaconInfo* beacon;
    if (isNew) {
        BeaconInfo newBeacon;
        newBeacon.macAddress = macKey;
        newBeacon.uuid = String(raw.uuid);
        newBeacon.major = raw.major;
        newBeacon.minor = raw.minor;
        newBeacon.txPower = raw.txPower;
        newBeacon.updateRSSI(raw.rssi);
        newBeacon.isPresent = true;
        
        _beacons[macKey] = newBeacon;
        beacon = &_beacons[macKey];
        
        Serial.printf("[BeaconService] New beacon: %s, RSSI=%d dBm\n",
                     raw.macAddress, raw.rssi);
    } else {
        beacon = &it->second;
        bool wasPresent = beacon->isPresent;
        
        // Update RSSI (with averaging)
        beacon->updateRSSI(raw.rssi);
        
        // Update presence based on averaged RSSI
        // Using averaged RSSI for hysteresis reduces false transitions
        beacon->isPresent = true;  // We saw it, so it's present
        
        // Notify callback if presence state changed
        if (!wasPresent && _callback) {
            _callback(*beacon);
        }
    }
    
    // CRITICAL: Always call callback during race (for lap detection)
    // Even for updates, not just new beacons!
    if (_raceMode && _callback) {
        _callback(*beacon);
    } else if (isNew && _callback) {
        // Outside race mode, only notify on new beacons
        _callback(*beacon);
    }
}

std::vector<BeaconInfo> BeaconService::getBeacons() const {
    std::vector<BeaconInfo> result;
    for (const auto& pair : _beacons) {
        result.push_back(pair.second);
    }
    return result;
}

BeaconInfo* BeaconService::getBeacon(const String& macAddress) {
    auto it = _beacons.find(macAddress);
    if (it != _beacons.end()) {
        return &it->second;
    }
    return nullptr;
}

BeaconInfo* BeaconService::getNearestBeacon() {
    BeaconInfo* nearest = nullptr;
    int8_t maxRSSI = -128;
    
    for (auto& pair : _beacons) {
        if (pair.second.avgRssi > maxRSSI) {
            maxRSSI = pair.second.avgRssi;
            nearest = &pair.second;
        }
    }
    
    return nearest;
}

float BeaconService::calculateDistance(int8_t rssi, int8_t txPower) {
    return BLEInterface::rssiToDistance(rssi, txPower);
}

