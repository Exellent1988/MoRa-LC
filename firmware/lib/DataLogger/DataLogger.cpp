#include "DataLogger.h"

DataLogger::DataLogger() 
    : initialized(false)
    , currentRaceFile("")
    , raceStartTime(0) {
}

DataLogger::~DataLogger() {
}

bool DataLogger::begin(uint8_t csPin) {
    Serial.printf("[DataLogger] Initializing SD card (CS Pin: %u)...\n", csPin);
    
    if (!SD.begin(csPin)) {
        Serial.println("[DataLogger] ERROR: SD card initialization failed");
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[DataLogger] ERROR: No SD card attached");
        return false;
    }
    
    Serial.print("[DataLogger] SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("[DataLogger] SD Card Size: %llu MB\n", cardSize);
    Serial.printf("[DataLogger] Free Space: %llu MB\n", getFreeSpace() / (1024 * 1024));
    
    // Erstelle /races Verzeichnis falls nicht vorhanden
    if (!SD.exists("/races")) {
        SD.mkdir("/races");
        Serial.println("[DataLogger] Created /races directory");
    }
    
    initialized = true;
    Serial.println("[DataLogger] Initialized successfully");
    return true;
}

bool DataLogger::isReady() {
    return initialized;
}

bool DataLogger::writeFile(const String& path, const String& data, bool append) {
    if (!initialized) {
        Serial.println("[DataLogger] ERROR: Not initialized");
        return false;
    }
    
    File file = SD.open(path.c_str(), append ? FILE_APPEND : FILE_WRITE);
    if (!file) {
        Serial.printf("[DataLogger] ERROR: Failed to open file: %s\n", path.c_str());
        return false;
    }
    
    size_t written = file.print(data);
    file.close();
    
    if (written == data.length()) {
        Serial.printf("[DataLogger] Written %u bytes to %s\n", written, path.c_str());
        return true;
    } else {
        Serial.printf("[DataLogger] ERROR: Write failed (%u/%u bytes)\n", written, data.length());
        return false;
    }
}

String DataLogger::readFile(const String& path) {
    if (!initialized) {
        return "";
    }
    
    File file = SD.open(path.c_str(), FILE_READ);
    if (!file) {
        Serial.printf("[DataLogger] ERROR: Failed to open file: %s\n", path.c_str());
        return "";
    }
    
    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();
    
    return content;
}

bool DataLogger::deleteFile(const String& path) {
    if (!initialized) {
        return false;
    }
    
    if (SD.remove(path.c_str())) {
        Serial.printf("[DataLogger] Deleted: %s\n", path.c_str());
        return true;
    } else {
        Serial.printf("[DataLogger] ERROR: Failed to delete: %s\n", path.c_str());
        return false;
    }
}

bool DataLogger::exists(const String& path) {
    return initialized && SD.exists(path.c_str());
}

bool DataLogger::logCSV(const String& filename, const String& data) {
    return writeFile(filename, data + "\n", true);
}

bool DataLogger::createCSVHeader(const String& filename, const String& header) {
    // Prüfe ob Datei bereits existiert
    if (exists(filename)) {
        Serial.printf("[DataLogger] File already exists: %s\n", filename.c_str());
        return true;  // Header bereits vorhanden
    }
    
    return writeFile(filename, header + "\n", false);
}

bool DataLogger::startNewRace(const String& raceName) {
    if (!initialized) {
        return false;
    }
    
    currentRaceFile = generateRaceFilename(raceName);
    raceStartTime = millis();
    
    Serial.printf("[DataLogger] New race started: %s\n", currentRaceFile.c_str());
    
    // CSV Header erstellen
    String header = "Team ID,Team Name,Lap Number,Timestamp (ms),Duration (ms),Time of Day";
    return createCSVHeader(currentRaceFile, header);
}

bool DataLogger::logLap(uint8_t teamId, const String& teamName, uint16_t lapNumber, 
                        uint32_t timestamp, uint32_t duration) {
    if (!initialized || currentRaceFile.isEmpty()) {
        Serial.println("[DataLogger] ERROR: No active race");
        return false;
    }
    
    // Time of Day formatieren (HH:MM:SS)
    uint32_t seconds = (timestamp - raceStartTime) / 1000;
    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    
    char timeStr[16];
    sprintf(timeStr, "%02lu:%02lu:%02lu", hours, minutes, secs);
    
    // CSV Zeile erstellen
    String csvLine = String(teamId) + ",";
    csvLine += teamName + ",";
    csvLine += String(lapNumber) + ",";
    csvLine += String(timestamp) + ",";
    csvLine += String(duration) + ",";
    csvLine += String(timeStr);
    
    return logCSV(currentRaceFile, csvLine);
}

bool DataLogger::finishRace() {
    if (currentRaceFile.isEmpty()) {
        return false;
    }
    
    Serial.printf("[DataLogger] Race finished: %s\n", currentRaceFile.c_str());
    
    // Race-Summary schreiben
    String summaryFile = currentRaceFile;
    summaryFile.replace(".csv", "_summary.txt");
    
    uint32_t raceDuration = millis() - raceStartTime;
    String summary = "Race finished\n";
    summary += "Duration: " + String(raceDuration / 1000) + " seconds\n";
    summary += "File: " + currentRaceFile + "\n";
    
    writeFile(summaryFile, summary, false);
    
    currentRaceFile = "";
    raceStartTime = 0;
    
    return true;
}

uint64_t DataLogger::getFreeSpace() {
    if (!initialized) {
        return 0;
    }
    return SD.totalBytes() - SD.usedBytes();
}

uint64_t DataLogger::getUsedSpace() {
    if (!initialized) {
        return 0;
    }
    return SD.usedBytes();
}

String DataLogger::listFiles(const String& dir) {
    if (!initialized) {
        return "";
    }
    
    File root = SD.open(dir.c_str());
    if (!root || !root.isDirectory()) {
        return "";
    }
    
    String fileList = "";
    File file = root.openNextFile();
    
    while (file) {
        if (file.isDirectory()) {
            fileList += "[DIR] " + String(file.name()) + "\n";
        } else {
            fileList += String(file.name()) + " (" + String(file.size()) + " bytes)\n";
        }
        file = root.openNextFile();
    }
    
    return fileList;
}

String DataLogger::getCurrentRaceFile() {
    return currentRaceFile;
}

// ============================================================
// Private Helper
// ============================================================

String DataLogger::sanitizeFilename(const String& name) {
    String clean = name;
    
    // Ersetze ungültige Zeichen
    clean.replace(" ", "_");
    clean.replace("/", "_");
    clean.replace("\\", "_");
    clean.replace(":", "_");
    clean.replace("*", "_");
    clean.replace("?", "_");
    clean.replace("\"", "_");
    clean.replace("<", "_");
    clean.replace(">", "_");
    clean.replace("|", "_");
    
    // Maximal 30 Zeichen
    if (clean.length() > 30) {
        clean = clean.substring(0, 30);
    }
    
    return clean;
}

String DataLogger::generateRaceFilename(const String& raceName) {
    // Format: /races/YYYYMMDD_HHMMSS_RaceName.csv
    
    // Timestamp (falls RTC verfügbar, sonst millis())
    uint32_t timestamp = millis() / 1000;
    uint32_t hours = (timestamp / 3600) % 24;
    uint32_t minutes = (timestamp / 60) % 60;
    uint32_t seconds = timestamp % 60;
    
    // Datum generieren (Platzhalter ohne RTC)
    char filename[64];
    sprintf(filename, "/races/%08lu_%02lu%02lu%02lu_%s.csv",
            timestamp, hours, minutes, seconds,
            sanitizeFilename(raceName).c_str());
    
    return String(filename);
}



