#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>

/**
 * System Management
 * Handles initialization, watchdog, and system-level operations
 */
class SystemManager {
public:
    SystemManager();
    ~SystemManager();
    
    // Initialization
    bool initialize();
    bool isReady() const { return _initialized; }
    
    // Watchdog
    void feedWatchdog();
    void enableWatchdog(uint32_t timeoutMs = 30000);
    void disableWatchdog();
    
    // System info
    uint32_t getFreeHeap();
    uint32_t getLargestFreeBlock();
    uint8_t getHeapFragmentation();
    
    // Performance monitoring
    void updateFrameTime(uint32_t frameTimeMs);
    uint32_t getAverageFrameTime() const { return _avgFrameTime; }
    uint32_t getMaxFrameTime() const { return _maxFrameTime; }
    
private:
    bool _initialized;
    uint32_t _avgFrameTime;
    uint32_t _maxFrameTime;
    uint32_t _frameTimeSum;
    uint32_t _frameCount;
};

#endif // SYSTEM_H

