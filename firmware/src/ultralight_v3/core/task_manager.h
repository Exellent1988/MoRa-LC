#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

/**
 * Task Manager
 * FreeRTOS task management for UI, BLE, and logging
 */

// Task priorities
#define TASK_PRIORITY_UI 2
#define TASK_PRIORITY_BLE 3
#define TASK_PRIORITY_LOGGING 1
#define TASK_PRIORITY_IDLE 0

// Task stack sizes (in words, 32-bit)
#define TASK_STACK_SIZE_UI 4096
#define TASK_STACK_SIZE_BLE 4096
#define TASK_STACK_SIZE_LOGGING 2048

/**
 * Task Manager
 * Manages FreeRTOS tasks for different subsystems
 */
class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    
    // Initialization
    bool begin();
    void end();
    
    // Task creation
    bool createUITask(void (*taskFunction)(void*), void* parameter = nullptr);
    bool createBLETask(void (*taskFunction)(void*), void* parameter = nullptr);
    bool createLoggingTask(void (*taskFunction)(void*), void* parameter = nullptr);
    
    // Task management
    void suspendUITask();
    void resumeUITask();
    void suspendBLETask();
    void resumeBLETask();
    
    // Status
    bool isUITaskRunning() const;
    bool isBLETaskRunning() const;
    bool isLoggingTaskRunning() const;
    
private:
    TaskHandle_t _uiTaskHandle;
    TaskHandle_t _bleTaskHandle;
    TaskHandle_t _loggingTaskHandle;
    bool _initialized;
};

#endif // TASK_MANAGER_H

