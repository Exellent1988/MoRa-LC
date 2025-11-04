#include "memory_manager.h"
#include <Arduino.h>

size_t MemoryManager::_minFreeHeap = SIZE_MAX;

MemoryManager::MemoryManager() {
    _minFreeHeap = getFreeHeap();
}

MemoryManager::~MemoryManager() {
}

size_t MemoryManager::getFreeHeap() {
    return esp_get_free_heap_size();
}

size_t MemoryManager::getLargestFreeBlock() {
    return heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
}

size_t MemoryManager::getMinFreeHeap() {
    return _minFreeHeap;
}

size_t MemoryManager::getTotalHeap() {
    return ESP.getHeapSize();
}

void* MemoryManager::allocate(size_t size, uint32_t caps) {
    void* ptr = heap_caps_malloc(size, caps);
    if (ptr) {
        size_t free = getFreeHeap();
        if (free < _minFreeHeap) {
            _minFreeHeap = free;
        }
    }
    return ptr;
}

void MemoryManager::deallocate(void* ptr) {
    if (ptr) {
        heap_caps_free(ptr);
    }
}

void MemoryManager::printStats() {
    Serial.printf("[Memory] Free: %zu bytes, Largest: %zu bytes, Min: %zu bytes, Total: %zu bytes\n",
                 getFreeHeap(), getLargestFreeBlock(), getMinFreeHeap(), getTotalHeap());
}

bool MemoryManager::checkLowMemory(size_t threshold) {
    return getFreeHeap() < threshold;
}

void MemoryManager::optimize() {
    // Force garbage collection if available
    // ESP32 doesn't have automatic GC, but we can check memory fragmentation
    size_t free = getFreeHeap();
    size_t largest = getLargestFreeBlock();
    
    if (largest < free * 0.5) {
        // Significant fragmentation detected
        Serial.println("[Memory] Warning: High memory fragmentation detected");
    }
}

