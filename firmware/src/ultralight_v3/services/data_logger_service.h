#ifndef DATA_LOGGER_SERVICE_H
#define DATA_LOGGER_SERVICE_H

#include <Arduino.h>
#include "../core/config.h"
#include "../hardware/sd_card.h"
#include <SD.h>
#include <queue>
#include <string>
#include <vector>

/**
 * Data Logger Service
 * Async SD card logging for race data
 */

struct LogEntry {
    uint32_t timestamp;
    uint8_t teamId;
    String teamName;
    uint32_t lapCount;
    int8_t rssi;
    String data;  // Formatted log line
    
    LogEntry() : timestamp(0), teamId(0), lapCount(0), rssi(0) {}
};

/**
 * Data Logger Service
 * Handles async logging to SD card
 */
class DataLoggerService {
public:
    DataLoggerService();
    ~DataLoggerService();
    
    // Initialization
    bool begin(SDCard* sdCard);
    void end();
    
    // Logging operations
    bool logLap(uint8_t teamId, const String& teamName, uint32_t lapCount, int8_t rssi);
    bool logRaceStart(const String& raceName);
    bool logRaceEnd();
    
    // Queue management
    void update();  // Call in loop to process queue
    size_t getQueueSize() const { return _logQueue.size(); }
    
    // File management
    bool createRaceFile(const String& raceName);
    void closeRaceFile();
    
    // Race history
    std::vector<String> getRaceFiles(uint8_t maxFiles = 10);
    String readFile(const String& path);
    bool fileExists(const String& path);
    String getCurrentRaceFile() const { return _currentFilePath; }
    
    // Status
    bool isReady() const { return _initialized && _sdCard && _sdCard->isReady(); }
    
private:
    SDCard* _sdCard;
    bool _initialized;
    std::queue<LogEntry> _logQueue;
    File _raceFile;  // Arduino SD File type
    String _currentRaceName;
    String _currentFilePath;
    uint32_t _lastFlush;
    uint32_t _raceStartMs;
    
    // Helper methods
    bool writeEntry(const LogEntry& entry);
    void flushQueue();
    String formatLogLine(const LogEntry& entry) const;
};

#endif // DATA_LOGGER_SERVICE_H

