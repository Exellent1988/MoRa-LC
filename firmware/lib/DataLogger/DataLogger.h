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
    bool selfTest();   // schreibt/liest/loescht eine Testdatei -> SD wirklich nutzbar?
    
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

    // Roh-Daten (jedes empfangene Paket) fuer Offline-Replay/Debugging.
    // Wird im RAM gepuffert; serviceRaw() regelmaessig aus loop() aufrufen.
    void logRaw(uint32_t ts, const char* mac, int8_t rssi);
    void serviceRaw(bool force = false);

    // Diagnose
    uint32_t getWriteErrors() const { return writeErrors; }
    uint32_t getLapsLogged() const { return lapsLogged; }
    bool hasError() const { return lastWriteFailed; }
    String getRaceFileList(uint8_t maxFiles = 10);  // Get list of race files
    
private:
    bool initialized;
    String currentRaceFile;
    String currentRawFile;
    uint32_t raceStartTime;
    uint8_t csPin;
    File lapFile;   // waehrend des Rennens offen (kein open/close pro Runde)
    File rawFile;
    char rawBuf[2048];
    size_t rawLen;
    uint32_t lastRawFlush;
    uint32_t writeErrors;
    uint32_t lapsLogged;
    bool lastWriteFailed;

    bool recover();                       // SD neu initialisieren + Dateien wieder oeffnen
    bool appendLine(File& f, const char* path, const String& line);
    
    // Helper
    String sanitizeFilename(const String& name);
    String generateRaceFilename(const String& raceName);
    void deleteAllFiles(const String& dirPath);
};

#endif // DATA_LOGGER_H



