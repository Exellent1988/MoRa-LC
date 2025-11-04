#include "data_logger_service.h"
#include "../hardware/sd_card.h"
#include "../core/config.h"
#include <SdFat.h>
#include <Arduino.h>

// SdFat instance (global, shared with SDCard class if needed)
SdFat sdFat;

DataLoggerService::DataLoggerService()
    : _sdCard(nullptr)
    , _initialized(false)
    , _lastFlush(0) {
}

DataLoggerService::~DataLoggerService() {
    end();
}

bool DataLoggerService::begin(SDCard* sdCard) {
    if (!sdCard || !sdCard->isReady()) {
        Serial.println("[DataLogger] ERROR: SD card not ready");
        return false;
    }
    
    _sdCard = sdCard;
    
    // Initialize SdFat if not already initialized
    if (!sdFat.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
        Serial.println("[DataLogger] ERROR: SdFat initialization failed");
        return false;
    }
    
    // Ensure /races directory exists
    if (!sdFat.exists("/races")) {
        if (!sdFat.mkdir("/races")) {
            Serial.println("[DataLogger] ERROR: Failed to create /races directory");
            return false;
        }
        Serial.println("[DataLogger] Created /races directory");
    }
    
    _initialized = true;
    
    Serial.println("[DataLogger] Initialized");
    return true;
}

void DataLoggerService::end() {
    flushQueue();
    closeRaceFile();
    _initialized = false;
}

bool DataLoggerService::createRaceFile(const String& raceName) {
    if (!isReady()) return false;
    
    closeRaceFile(); // Close previous file if open
    
    _currentRaceName = raceName;
    
    // Create filename with timestamp
    char filename[64];
    snprintf(filename, sizeof(filename), "/races/race_%lu.csv", millis());
    
    // Open file for writing (create if not exists, truncate if exists)
    if (!_raceFile.open(&sdFat, filename, O_WRONLY | O_CREAT | O_TRUNC)) {
        Serial.printf("[DataLogger] ERROR: Failed to create race file: %s\n", filename);
        return false;
    }
    
    Serial.printf("[DataLogger] Race file created: %s\n", filename);
    
    return true;
}

void DataLoggerService::closeRaceFile() {
    if (_raceFile.isOpen()) {
        _raceFile.close();
    }
    _currentRaceName = "";
}

bool DataLoggerService::logLap(uint8_t teamId, const String& teamName, uint32_t lapCount, int8_t rssi) {
    if (!isReady()) return false;
    
    LogEntry entry;
    entry.timestamp = millis();
    entry.teamId = teamId;
    entry.teamName = teamName;
    entry.lapCount = lapCount;
    entry.rssi = rssi;
    entry.data = formatLogLine(entry);
    
    _logQueue.push(entry);
    
    // Auto-flush if queue is large
    if (_logQueue.size() > 10) {
        flushQueue();
    }
    
    return true;
}

bool DataLoggerService::logRaceStart(const String& raceName) {
    if (!createRaceFile(raceName)) return false;
    
    // Log header
    LogEntry entry;
    entry.timestamp = millis();
    entry.data = "timestamp,teamId,teamName,lapCount,rssi\n";
    _logQueue.push(entry);
    
    return true;
}

bool DataLoggerService::logRaceEnd() {
    flushQueue();
    closeRaceFile();
    return true;
}

void DataLoggerService::update() {
    if (!isReady()) return;
    
    // Flush queue periodically (every 1 second)
    uint32_t now = millis();
    if (now - _lastFlush > 1000) {
        flushQueue();
        _lastFlush = now;
    }
}

void DataLoggerService::flushQueue() {
    while (!_logQueue.empty()) {
        const LogEntry& entry = _logQueue.front();
        writeEntry(entry);
        _logQueue.pop();
    }
}

bool DataLoggerService::writeEntry(const LogEntry& entry) {
    if (!_raceFile.isOpen()) return false;
    
    // Write to file
    _raceFile.print(entry.data.c_str());
    _raceFile.flush();  // Ensure data is written immediately
    Serial.printf("[DataLogger] %s", entry.data.c_str());
    
    return true;
}

String DataLoggerService::formatLogLine(const LogEntry& entry) const {
    char buf[128];
    snprintf(buf, sizeof(buf), "%lu,%u,%s,%u,%d\n",
             entry.timestamp, entry.teamId, entry.teamName.c_str(),
             entry.lapCount, entry.rssi);
    return String(buf);
}

