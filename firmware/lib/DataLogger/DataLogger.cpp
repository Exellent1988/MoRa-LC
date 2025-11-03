#include "DataLogger.h"

// For real SD card formatting
#ifdef ULTRALIGHT_V2
#include <SdFat.h>
SdFat SD_FAT;
#endif

DataLogger::DataLogger() 
    : initialized(false)
    , currentRaceFile("")
    , raceStartTime(0)
    , csPin(5) {  // Default CS pin
}

DataLogger::~DataLogger() {
}

bool DataLogger::begin(uint8_t csPin) {
    this->csPin = csPin;  // Store CS pin for later use (formatting)
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

bool DataLogger::formatSD() {
    Serial.println("[DataLogger] WARNING: Formatting SD card - ALL DATA WILL BE LOST!");
    Serial.println("[DataLogger] This will perform a REAL low-level format to FAT32!");
    
    // Close any open files
    currentRaceFile = "";
    
#ifdef ULTRALIGHT_V2
    // Use SDFat library for REAL low-level formatting to FAT32
    Serial.println("[DataLogger] Initializing SDFat library for REAL formatting...");
    
    // Use stored CS pin
    uint8_t csPin = this->csPin;
    Serial.printf("[DataLogger] Using CS pin: %u\n", csPin);
    
    // Initialize SDFat with SPI config
    SdSpiConfig config(csPin, DEDICATED_SPI, SD_SCK_MHZ(4));
    
    // Create SdCard object for low-level access
    SdSpiCard card;
    
    // Initialize card for formatting (low-level access)
    if (!card.begin(config)) {
        Serial.println("[DataLogger] ERROR: Failed to initialize SD card for formatting");
        Serial.println("[DataLogger] Card may not be inserted or defective");
        return false;
    }
    
    // Perform REAL low-level format to FAT32
    Serial.println("[DataLogger] ========================================");
    Serial.println("[DataLogger] Starting REAL low-level format to FAT32");
    Serial.println("[DataLogger] This will completely wipe the card!");
    Serial.println("[DataLogger] This may take 30-120 seconds...");
    Serial.println("[DataLogger] ========================================");
    
    // Initialize SDFat filesystem (may fail if card not formatted - that's OK)
    // We need the card initialized, not necessarily the filesystem
    if (!SD_FAT.begin(config)) {
        Serial.println("[DataLogger] Card initialized but filesystem not found - will format");
    }
    
    // Use SDFat's format() method for REAL FAT32 formatting
    // This creates a NEW FAT32 filesystem on the card (low-level format)
    Serial.println("[DataLogger] Formatting FAT32 filesystem...");
    Serial.println("[DataLogger] This performs a REAL low-level format!");
    
    // Format using SDFat's format() method
    // This creates a new FAT32 filesystem on the card
    if (!SD_FAT.format(&Serial)) {
        Serial.println("[DataLogger] ERROR: Format operation failed");
        Serial.println("[DataLogger] Card may be write-protected or defective");
        Serial.println("[DataLogger] Try formatting with external tool (FAT32)");
        card.end();
        SD_FAT.end();
        return false;
    }
    
    Serial.println("[DataLogger] ========================================");
    Serial.println("[DataLogger] Format complete! FAT32 filesystem created.");
    Serial.println("[DataLogger] ========================================");
    
    // Close SDFat and card
    SD_FAT.end();
    card.end();
    
    // Small delay for card to stabilize
    delay(500);
    
    // Reinitialize standard SD library to use formatted card
    Serial.println("[DataLogger] Reinitializing SD library...");
    if (!SD.begin(csPin)) {
        Serial.println("[DataLogger] ERROR: Failed to reinitialize SD after format");
        Serial.println("[DataLogger] Card formatted but library init failed");
        initialized = false;
        return false;
    }
    
    // Verify card is accessible
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[DataLogger] ERROR: SD card not accessible after format");
        initialized = false;
        return false;
    }
    
    // Recreate essential directories
    if (!SD.exists("/races")) {
        if (SD.mkdir("/races")) {
            Serial.println("[DataLogger] Created /races directory");
        } else {
            Serial.println("[DataLogger] WARNING: Failed to create /races directory");
        }
    }
    
    // Get card info
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    uint64_t freeSpace = getFreeSpace() / (1024 * 1024);
    
    Serial.println("[DataLogger] ========================================");
    Serial.println("[DataLogger] SD CARD FORMATTED SUCCESSFULLY!");
    Serial.printf("[DataLogger] Card type: %s\n", 
                  cardType == CARD_SDHC ? "SDHC" : 
                  cardType == CARD_SD ? "SDSC" : "UNKNOWN");
    Serial.printf("[DataLogger] Total size: %llu MB\n", cardSize);
    Serial.printf("[DataLogger] Free space: %llu MB\n", freeSpace);
    Serial.println("[DataLogger] ========================================");
    
    initialized = true;
    return true;
#else
    // Old variant: Only delete files (no real format available)
    Serial.println("[DataLogger] WARNING: Real formatting not available in old variant");
    Serial.println("[DataLogger] Deleting all files and directories instead...");
    
    deleteAllFiles("/");
    
    if (SD.exists("/races")) {
        SD.rmdir("/races");
    }
    
    if (!SD.exists("/races")) {
        SD.mkdir("/races");
    }
    
    Serial.println("[DataLogger] All files deleted (NOT a real format - use external tool)");
    return true;
#endif
}

void DataLogger::deleteAllFiles(const String& dirPath) {
    File dir = SD.open(dirPath.c_str());
    if (!dir) {
        Serial.printf("[DataLogger] WARNING: Cannot open directory: %s\n", dirPath.c_str());
        return;
    }
    
    if (!dir.isDirectory()) {
        // It's a file, not a directory - delete it
        dir.close();
        if (SD.remove(dirPath.c_str())) {
            Serial.printf("[DataLogger] Deleted file: %s\n", dirPath.c_str());
        }
        return;
    }
    
    // It's a directory - delete all files and subdirectories
    File file = dir.openNextFile();
    while (file) {
        String filePath = String(file.name());
        if (file.isDirectory()) {
            // Recursively delete subdirectory
            deleteAllFiles(filePath);
            // Remove empty directory
            SD.rmdir(filePath.c_str());
            Serial.printf("[DataLogger] Removed directory: %s\n", filePath.c_str());
        } else {
            // Delete file
            file.close();
            if (SD.remove(filePath.c_str())) {
                Serial.printf("[DataLogger] Deleted: %s\n", filePath.c_str());
            }
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
    char filename[64];
    sprintf(filename, "/races/%08lu_%02lu%02lu%02lu_%s.csv",
            timestamp, hours, minutes, seconds,
            sanitizeFilename(raceName).c_str());
    
    return String(filename);
}



