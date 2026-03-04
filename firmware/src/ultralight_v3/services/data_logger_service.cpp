#include "data_logger_service.h"
#include "../hardware/sd_card.h"
#include <SD.h>
#include <Arduino.h>

DataLoggerService::DataLoggerService()
    : _sdCard(nullptr)
    , _initialized(false)
    , _lastFlush(0)
    , _raceStartMs(0) {
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
    
    // Create filename: /races/race_<millis>_<name>.csv
    char filename[96];
    if (raceName.length() > 0) {
        String safeName = raceName;
        safeName.replace(" ", "_");
        safeName.replace("/", "");
        snprintf(filename, sizeof(filename), "/races/race_%lu_%s.csv", millis(), safeName.c_str());
    } else {
        snprintf(filename, sizeof(filename), "/races/race_%lu.csv", millis());
    }
    
    // Ensure /races directory exists
    if (!_sdCard->exists("/races")) {
        if (!_sdCard->mkdir("/races")) {
            Serial.println("[DataLogger] ERROR: Failed to create /races directory");
            return false;
        }
    }
    
    _currentFilePath = String(filename);
    
    _raceFile = SD.open(filename, FILE_WRITE);
    if (!_raceFile) {
        Serial.printf("[DataLogger] ERROR: Failed to create file: %s\n", filename);
        _currentFilePath = "";
        return false;
    }
    
    Serial.printf("[DataLogger] Race file created: %s\n", filename);
    return true;
}

void DataLoggerService::closeRaceFile() {
    if (_raceFile) {
        _raceFile.flush();
        _raceFile.close();
        Serial.println("[DataLogger] Race file closed");
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
    Serial.printf("[DataLogger] logRaceStart called: %s (ready=%d)\n", raceName.c_str(), isReady());
    if (!createRaceFile(raceName)) {
        Serial.println("[DataLogger] ERROR: createRaceFile failed");
        return false;
    }
    
    _raceStartMs = millis();
    
    LogEntry entry;
    entry.timestamp = _raceStartMs;
    entry.data = "Team ID,Team Name,Lap Number,Timestamp (ms),Duration (ms),Time of Day,RSSI\n";
    _logQueue.push(entry);
    
    Serial.println("[DataLogger] Race logging started");
    return true;
}

bool DataLoggerService::logRaceEnd() {
    Serial.printf("[DataLogger] logRaceEnd called (queue=%zu)\n", _logQueue.size());
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
    if (!_raceFile) return false;
    
    // Write to file
    size_t written = _raceFile.print(entry.data.c_str());
    _raceFile.flush();  // Ensure data is written to SD card
    
    if (written > 0) {
        Serial.printf("[DataLogger] Wrote: %s", entry.data.c_str());
        return true;
    } else {
        Serial.println("[DataLogger] ERROR: Failed to write entry");
        return false;
    }
}

std::vector<String> DataLoggerService::getRaceFiles(uint8_t maxFiles) {
    std::vector<String> files;
    if (!isReady()) return files;
    
    File dir = SD.open("/races");
    if (!dir || !dir.isDirectory()) return files;
    
    File entry = dir.openNextFile();
    while (entry) {
        String name = String(entry.name());
        if (name.endsWith(".csv")) {
            // Store full path
            if (!name.startsWith("/")) name = "/races/" + name;
            files.push_back(name);
        }
        entry = dir.openNextFile();
    }
    dir.close();
    
    // Sort newest first (higher millis number = newer)
    // Files are named race_<millis>.csv, so reverse alpha sort works
    for (int i = 0; i < (int)files.size() - 1; i++) {
        for (int j = i + 1; j < (int)files.size(); j++) {
            if (files[j] > files[i]) {
                String tmp = files[i];
                files[i] = files[j];
                files[j] = tmp;
            }
        }
    }
    
    if (files.size() > maxFiles) files.resize(maxFiles);
    return files;
}

String DataLoggerService::readFile(const String& path) {
    if (!isReady()) return "";
    
    File f = SD.open(path, FILE_READ);
    if (!f) return "";
    
    String content;
    size_t size = f.size();
    if (size > 8192) size = 8192;  // Limit to 8KB
    content.reserve(size);
    
    while (f.available()) {
        content += (char)f.read();
        if (content.length() >= 8192) break;
    }
    f.close();
    return content;
}

bool DataLoggerService::fileExists(const String& path) {
    if (!isReady()) return false;
    return SD.exists(path);
}

String DataLoggerService::formatLogLine(const LogEntry& entry) const {
    uint32_t elapsed = entry.timestamp - _raceStartMs;
    uint32_t totalSec = elapsed / 1000;
    uint32_t hours = totalSec / 3600;
    uint32_t minutes = (totalSec % 3600) / 60;
    uint32_t seconds = totalSec % 60;
    
    char buf[160];
    snprintf(buf, sizeof(buf), "%u,%s,%u,%lu,%lu,%02lu:%02lu:%02lu,%d\n",
             entry.teamId, entry.teamName.c_str(),
             entry.lapCount, entry.timestamp,
             elapsed, hours, minutes, seconds, entry.rssi);
    return String(buf);
}

