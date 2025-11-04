#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>
#include <SdFat.h>
#include "../core/config.h"

/**
 * SD Card Abstraction
 * Simple interface for SD card operations
 */
class SDCard {
public:
    SDCard();
    ~SDCard();
    
    // Initialization
    bool begin(uint8_t csPin = SD_CS_PIN);
    bool isReady() const { return _initialized; }
    
    // File operations
    bool exists(const String& path);
    bool mkdir(const String& path);
    bool remove(const String& path);
      FsFile open(const String& path, oflag_t oflag = O_RDWR | O_CREAT);
      void close(FsFile& file);
    
    // Info
    uint64_t getTotalSize();
    uint64_t getFreeSpace();
    uint64_t getUsedSpace();
    
private:
    bool _initialized;
    uint8_t _csPin;
      SdFat32 _sd;
};

#endif // SD_CARD_H

