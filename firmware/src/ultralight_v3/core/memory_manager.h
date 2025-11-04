#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <Arduino.h>
#include <esp_heap_caps.h>

/**
 * Memory Manager
 * Optimized memory allocation and monitoring
 */

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();
    
    // Memory statistics
    static size_t getFreeHeap();
    static size_t getLargestFreeBlock();
    static size_t getMinFreeHeap();
    static size_t getTotalHeap();
    
    // Memory allocation (with tracking)
    static void* allocate(size_t size, uint32_t caps = MALLOC_CAP_DEFAULT);
    static void deallocate(void* ptr);
    
    // Memory monitoring
    static void printStats();
    static bool checkLowMemory(size_t threshold = 50000);  // Default: 50KB
    
    // Memory optimization
    static void optimize();
    
private:
    static size_t _minFreeHeap;
};

#endif // MEMORY_MANAGER_H

