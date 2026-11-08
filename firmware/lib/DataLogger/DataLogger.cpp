#include "DataLogger.h"

DataLogger::DataLogger() 
    : initialized(false)
    , currentRaceFile("")
    , currentRawFile("")
    , raceStartTime(0)
    , csPin(5)
    , rawLen(0)
    , lastRawFlush(0)
    , writeErrors(0)
    , lapsLogged(0)
    , lastWriteFailed(false) {
}

DataLogger::~DataLogger() {
}

bool DataLogger::begin(uint8_t csPin) {
    Serial.printf("[DataLogger] Initializing SD card (CS Pin: %u)...\n", csPin);
    this->csPin = csPin;
    
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

bool DataLogger::selfTest() {
    if (!initialized) return false;
    const char* path = "/races/.sdtest";
    File f = SD.open(path, FILE_WRITE);
    if (!f) return false;
    size_t n = f.print("ok");
    f.close();
    bool ok = (n == 2);
    f = SD.open(path, FILE_READ);
    if (!f) return false;
    ok = ok && f.available() == 2;
    f.close();
    SD.remove(path);
    Serial.printf("[DataLogger] Self-test: %s\n", ok ? "OK" : "FAILED");
    return ok;
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
    
    // Alte Handles schliessen (falls vorheriges Rennen nicht sauber beendet wurde)
    if (lapFile) lapFile.close();
    if (rawFile) rawFile.close();

    String base = generateRaceFilename(raceName);  // ".../name.csv"
    base.replace(".csv", "");
    // Keine bestehende Datei weiterschreiben (millis()-basierte Namen kollidieren nach Reboot)
    String path = base + ".csv";
    for (int i = 2; SD.exists(path.c_str()) && i < 100; i++) {
        path = base + "_" + String(i) + ".csv";
    }
    currentRaceFile = path;
    currentRawFile = path;
    currentRawFile.replace(".csv", "_raw.csv");
    raceStartTime = millis();
    rawLen = 0;
    lastRawFlush = millis();
    lapsLogged = 0;
    writeErrors = 0;
    lastWriteFailed = false;
    
    Serial.printf("[DataLogger] New race started: %s\n", currentRaceFile.c_str());
    
    lapFile = SD.open(currentRaceFile.c_str(), FILE_WRITE);
    rawFile = SD.open(currentRawFile.c_str(), FILE_WRITE);
    if (!lapFile) {
        Serial.println("[DataLogger] ERROR: cannot open lap file");
        lastWriteFailed = true;
        writeErrors++;
        return false;
    }
    lapFile.println("Team ID,Team Name,Lap Number,Timestamp (ms),Duration (ms),Time of Day");
    lapFile.flush();
    if (rawFile) {
        rawFile.println("ms,mac,rssi");
        rawFile.flush();
    }
    return true;
}

bool DataLogger::recover() {
    Serial.println("[DataLogger] Recovering SD card...");
    if (lapFile) lapFile.close();
    if (rawFile) rawFile.close();
    SD.end();
    if (!SD.begin(csPin)) {
        Serial.println("[DataLogger] Recover failed: SD.begin");
        return false;
    }
    if (!currentRaceFile.isEmpty()) lapFile = SD.open(currentRaceFile.c_str(), FILE_APPEND);
    if (!currentRawFile.isEmpty()) rawFile = SD.open(currentRawFile.c_str(), FILE_APPEND);
    return (bool)lapFile;
}

bool DataLogger::appendLine(File& f, const char* path, const String& line) {
    for (int attempt = 0; attempt < 2; attempt++) {
        if (f) {
            size_t n = f.println(line);
            f.flush();
            if (n >= line.length()) {
                lastWriteFailed = false;
                return true;
            }
        }
        writeErrors++;
        lastWriteFailed = true;
        Serial.printf("[DataLogger] Write failed (%s), attempt %d\n", path, attempt + 1);
        if (attempt == 0 && !recover()) break;
    }
    return false;
}

bool DataLogger::logLap(uint8_t teamId, const String& teamName, uint16_t lapNumber, 
                        uint32_t timestamp, uint32_t duration) {
    if (!initialized || currentRaceFile.isEmpty()) {
        Serial.println("[DataLogger] ERROR: No active race");
        return false;
    }
    
    // Zeit seit Rennstart (HH:MM:SS)
    uint32_t seconds = (timestamp - raceStartTime) / 1000;
    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu",
             (unsigned long)hours, (unsigned long)minutes, (unsigned long)secs);
    
    String csvLine = String(teamId) + ",";
    csvLine += teamName + ",";
    csvLine += String(lapNumber) + ",";
    csvLine += String(timestamp) + ",";
    csvLine += String(duration) + ",";
    csvLine += String(timeStr);
    
    bool ok = appendLine(lapFile, currentRaceFile.c_str(), csvLine);
    if (ok) lapsLogged++;
    return ok;
}

void DataLogger::logRaw(uint32_t ts, const char* mac, int8_t rssi) {
    if (!initialized || currentRawFile.isEmpty()) return;
    if (rawLen + 40 >= sizeof(rawBuf)) serviceRaw(true);
    int n = snprintf(rawBuf + rawLen, sizeof(rawBuf) - rawLen, "%lu,%s,%d\n",
                     (unsigned long)ts, mac, (int)rssi);
    if (n > 0) rawLen += n;
}

void DataLogger::serviceRaw(bool force) {
    if (rawLen == 0) return;
    if (!force && millis() - lastRawFlush < 1000) return;
    lastRawFlush = millis();
    if (rawFile) {
        size_t n = rawFile.write((const uint8_t*)rawBuf, rawLen);
        rawFile.flush();
        if (n != rawLen) {
            writeErrors++;
            lastWriteFailed = true;
        }
    }
    rawLen = 0;  // Rohdaten sind "best effort": bei Fehler nicht endlos puffern
}

bool DataLogger::finishRace() {
    if (currentRaceFile.isEmpty()) {
        return false;
    }
    
    serviceRaw(true);
    if (lapFile) lapFile.close();
    if (rawFile) rawFile.close();
    
    Serial.printf("[DataLogger] Race finished: %s\n", currentRaceFile.c_str());
    
    String summaryFile = currentRaceFile;
    summaryFile.replace(".csv", "_summary.txt");
    
    uint32_t raceDuration = millis() - raceStartTime;
    String summary = "Race finished\n";
    summary += "Duration: " + String(raceDuration / 1000) + " seconds\n";
    summary += "Laps logged: " + String(lapsLogged) + "\n";
    summary += "Write errors: " + String(writeErrors) + "\n";
    summary += "File: " + currentRaceFile + "\n";
    
    writeFile(summaryFile, summary, false);
    
    currentRaceFile = "";
    currentRawFile = "";
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

bool DataLogger::formatSD() {
    if (!initialized) {
        Serial.println("[DataLogger] ERROR: SD card not initialized");
        return false;
    }
    
    Serial.println("[DataLogger] WARNING: Formatting SD card - ALL DATA WILL BE LOST!");
    
    // Close any open files
    if (lapFile) lapFile.close();
    if (rawFile) rawFile.close();
    currentRaceFile = "";
    currentRawFile = "";
    
    // Note: ESP32 SD library doesn't support format directly
    // We need to use SDFat library or delete all files
    // For now, we'll delete all files in /races directory
    
    File root = SD.open("/");
    if (!root) {
        Serial.println("[DataLogger] ERROR: Cannot open root directory");
        return false;
    }
    
    // Delete all files recursively
    deleteAllFiles("/");
    
    // Recreate races directory
    if (!SD.exists("/races")) {
        SD.mkdir("/races");
    }
    
    Serial.println("[DataLogger] SD card 'formatted' (all files deleted)");
    return true;
}

void DataLogger::deleteAllFiles(const String& dirPath) {
    File dir = SD.open(dirPath.c_str());
    if (!dir || !dir.isDirectory()) {
        return;
    }
    
    File file = dir.openNextFile();
    while (file) {
        String filePath = String(file.name());
        if (file.isDirectory()) {
            deleteAllFiles(filePath);
            SD.rmdir(filePath.c_str());
        } else {
            SD.remove(filePath.c_str());
            Serial.printf("[DataLogger] Deleted: %s\n", filePath.c_str());
        }
        file = dir.openNextFile();
    }
    dir.close();
}

String DataLogger::getRaceFileList(uint8_t maxFiles) {
    if (!initialized) {
        return "";
    }
    
    String fileList = "";
    File dir = SD.open("/races");
    if (!dir || !dir.isDirectory()) {
        return "";
    }
    
    // Collect all CSV files with their timestamps
    struct FileInfo {
        String name;
        unsigned long timestamp;
    };
    
    // Use simple array (max 20 files)
    FileInfo files[20];
    uint8_t fileCount = 0;
    
    File file = dir.openNextFile();
    while (file && fileCount < 20) {
        String fileName = String(file.name());
        if (fileName.endsWith(".csv")) {
            files[fileCount].name = fileName;
            files[fileCount].timestamp = file.getLastWrite();
            fileCount++;
        }
        file = dir.openNextFile();
    }
    dir.close();
    
    // Sort by timestamp (newest first) - simple bubble sort
    for (uint8_t i = 0; i < fileCount - 1; i++) {
        for (uint8_t j = 0; j < fileCount - i - 1; j++) {
            if (files[j].timestamp < files[j + 1].timestamp) {
                FileInfo temp = files[j];
                files[j] = files[j + 1];
                files[j + 1] = temp;
            }
        }
    }
    
    // Build result list
    uint8_t count = (fileCount < maxFiles) ? fileCount : maxFiles;
    for (uint8_t i = 0; i < count; i++) {
        if (i > 0) fileList += "\n";
        fileList += files[i].name;
    }
    
    return fileList;
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
    char filename[96];
    snprintf(filename, sizeof(filename), "/races/%08lu_%02lu%02lu%02lu_%.40s.csv",
             (unsigned long)timestamp, (unsigned long)hours, (unsigned long)minutes,
             (unsigned long)seconds, sanitizeFilename(raceName).c_str());
    
    return String(filename);
}



