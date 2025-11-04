#include "system.h"
#include "config.h"
#include <esp_task_wdt.h>

SystemManager::SystemManager()
    : _initialized(false)
    , _avgFrameTime(0)
    , _maxFrameTime(0)
    , _frameTimeSum(0)
    , _frameCount(0) {
}

SystemManager::~SystemManager() {
    disableWatchdog();
}

bool SystemManager::initialize() {
    Serial.begin(115200);
    delay(100);
    
    Serial.println("\n=================================");
    Serial.println(DEVICE_NAME " v" VERSION);
    Serial.println("=================================\n");
    
    // Enable watchdog
    enableWatchdog(30000);  // 30 second timeout
    
    _initialized = true;
    Serial.println("[System] Initialized successfully");
    return true;
}

void SystemManager::feedWatchdog() {
    if (_initialized) {
        esp_task_wdt_reset();
    }
}

void SystemManager::enableWatchdog(uint32_t timeoutMs) {
    // ESP32 Task Watchdog: timeout in seconds
    // Use longer timeout for serial-only mode (BLE scanning can be intensive)
    uint32_t timeoutSec = timeoutMs / 1000;
    if (timeoutSec < 60) timeoutSec = 60;  // Minimum 60 seconds
    
    esp_task_wdt_init(timeoutSec, true);  // true = panic on timeout
    esp_task_wdt_add(NULL);  // Add current task (loopTask)
    Serial.printf("[System] Watchdog enabled (%lu ms / %lu s)\n", timeoutMs, timeoutSec);
}

void SystemManager::disableWatchdog() {
    esp_task_wdt_delete(NULL);
    esp_task_wdt_deinit();
    Serial.println("[System] Watchdog disabled");
}

uint32_t SystemManager::getFreeHeap() {
    return ESP.getFreeHeap();
}

uint32_t SystemManager::getLargestFreeBlock() {
    return ESP.getMaxAllocHeap();
}

uint8_t SystemManager::getHeapFragmentation() {
    return 100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize());
}

void SystemManager::updateFrameTime(uint32_t frameTimeMs) {
    _frameTimeSum += frameTimeMs;
    _frameCount++;
    
    if (frameTimeMs > _maxFrameTime) {
        _maxFrameTime = frameTimeMs;
    }
    
    // Calculate average every 60 frames
    if (_frameCount >= 60) {
        _avgFrameTime = _frameTimeSum / _frameCount;
        _frameTimeSum = 0;
        _frameCount = 0;
        
        // Warn if frame time is too high
        if (_avgFrameTime > FRAME_TIME_MS) {
            Serial.printf("[System] WARNING: Average frame time %lu ms (target: %d ms)\n", 
                         _avgFrameTime, FRAME_TIME_MS);
        }
    }
}

