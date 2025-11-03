#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>
#include <SD.h>
#include <FS.h>

/**
 * Data Logger für SD-Karte
 * 
 * Loggt Rennendaten, Rundenzeiten, Telemetrie auf SD-Karte
 * Beide Varianten (FullBlown & UltraLight) nutzen diese Library
 */

class DataLogger {
public:
    DataLogger();
    ~DataLogger();
    
    // Initialisierung
    bool begin(uint8_t csPin = 5);
    bool isReady();
    
    // File Operations
    bool writeFile(const String& path, const String& data, bool append = true);
    String readFile(const String& path);
    bool deleteFile(const String& path);
    bool exists(const String& path);
    
    // CSV Logging
    bool logCSV(const String& filename, const String& data);
    bool createCSVHeader(const String& filename, const String& header);
    
    // Race Data
    bool startNewRace(const String& raceName);
    bool logLap(uint8_t teamId, const String& teamName, uint16_t lapNumber, 
                uint32_t timestamp, uint32_t duration);
    bool finishRace();
    
    // Utility
    uint64_t getFreeSpace();
    uint64_t getUsedSpace();
    String listFiles(const String& dir = "/");
    bool formatSD();  // Format SD card to FAT32
    
    // Export
    String getCurrentRaceFile();
    String getRaceFileList(uint8_t maxFiles = 10);  // Get list of race files
    
private:
    bool initialized;
    String currentRaceFile;
    uint32_t raceStartTime;
    
    // Helper
    String sanitizeFilename(const String& name);
    String generateRaceFilename(const String& raceName);
    void deleteAllFiles(const String& dirPath);
};

#endif // DATA_LOGGER_H



