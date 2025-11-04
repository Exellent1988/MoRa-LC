#include "sd_card.h"
#include <SPI.h>

SDCard::SDCard() : _initialized(false), _csPin(SD_CS_PIN), _sd() {
}

SDCard::~SDCard() {
}

bool SDCard::begin(uint8_t csPin) {
    _csPin = csPin;
    
    Serial.println("[SD] Initializing...");
    
    SdSpiConfig spiConfig(csPin, DEDICATED_SPI, SD_SCK_MHZ(4));
    if (!_sd.begin(spiConfig)) {
        Serial.println("[SD] ERROR: Initialization failed");
        _initialized = false;
        return false;
    }
    
    if (!_sd.card()) {
        Serial.println("[SD] ERROR: No SD card found");
        _initialized = false;
        return false;
    }
    
    uint8_t cardType = _sd.card()->type();
    const char* typeStr = "UNKNOWN";
    if (cardType == SD_CARD_TYPE_SD1) {
        typeStr = "SDSC";
    } else if (cardType == SD_CARD_TYPE_SD2) {
        typeStr = "SDHC";
    } else if (cardType == SD_CARD_TYPE_SDHC) {
        typeStr = "SDHC/SDXC";
    }
    Serial.printf("[SD] Card Type: %s\n", typeStr);
    
    uint64_t cardSize = (_sd.card()->sectorCount() * 512ULL) / (1024ULL * 1024ULL);
    Serial.printf("[SD] Card Size: %llu MB\n", cardSize);
    
    // Ensure /races directory exists
    if (!_sd.exists("/races")) {
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
    return _sd.exists(path.c_str());
}

bool SDCard::mkdir(const String& path) {
    if (!_initialized) return false;
    return _sd.mkdir(path.c_str());
}

bool SDCard::remove(const String& path) {
    if (!_initialized) return false;
    return _sd.remove(path.c_str());
}

FsFile SDCard::open(const String& path, oflag_t oflag) {
    if (!_initialized) return FsFile();
    return _sd.open(path.c_str(), oflag);
}

void SDCard::close(FsFile& file) {
    if (file) {
        file.close();
    }
}

uint64_t SDCard::getTotalSize() {
    if (!_initialized || !_sd.card()) return 0;
    return _sd.card()->sectorCount() * 512ULL;
}

uint64_t SDCard::getFreeSpace() {
    // TODO: Implement precise free space calculation using SdFat volume APIs if needed
    return 0;
}

uint64_t SDCard::getUsedSpace() {
    uint64_t total = getTotalSize();
    uint64_t free = getFreeSpace();
    if (total == 0 || free == 0 || free > total) {
        return 0;
    }
    return total - free;
}

