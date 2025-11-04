#include "sd_card.h"
#include <SD.h>
#include <SPI.h>

SDCard::SDCard() : _initialized(false), _csPin(SD_CS_PIN) {
}

SDCard::~SDCard() {
}

bool SDCard::begin(uint8_t csPin) {
    _csPin = csPin;
    
    Serial.println("[SD] Initializing...");
    
    if (!SD.begin(csPin, SPI, SD_SPI_FREQ)) {
        Serial.println("[SD] ERROR: Initialization failed");
        _initialized = false;
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[SD] ERROR: No SD card found");
        _initialized = false;
        return false;
    }
    
    Serial.printf("[SD] Card Type: %s\n", 
                 cardType == CARD_MMC ? "MMC" :
                 cardType == CARD_SD ? "SDSC" :
                 cardType == CARD_SDHC ? "SDHC" : "UNKNOWN");
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("[SD] Card Size: %llu MB\n", cardSize);
    
    // Ensure /races directory exists
    if (!SD.exists("/races")) {
        if (mkdir("/races")) {
            Serial.println("[SD] Created /races directory");
        }
    }
    
    _initialized = true;
    Serial.println("[SD] Initialized successfully");
    return true;
}

bool SDCard::exists(const String& path) {
    if (!_initialized) return false;
    return SD.exists(path);
}

bool SDCard::mkdir(const String& path) {
    if (!_initialized) return false;
    return SD.mkdir(path);
}

bool SDCard::remove(const String& path) {
    if (!_initialized) return false;
    return SD.remove(path);
}

uint64_t SDCard::getTotalSize() {
    if (!_initialized) return 0;
    return SD.cardSize();
}

uint64_t SDCard::getFreeSpace() {
    if (!_initialized) return 0;
    return SD.totalBytes() - SD.usedBytes();
}

uint64_t SDCard::getUsedSpace() {
    if (!_initialized) return 0;
    return SD.usedBytes();
}

