#include "task_manager.h"
#include <Arduino.h>

TaskManager::TaskManager()
    : _uiTaskHandle(nullptr)
    , _bleTaskHandle(nullptr)
    , _loggingTaskHandle(nullptr)
    , _initialized(false) {
}

TaskManager::~TaskManager() {
    end();
}

bool TaskManager::begin() {
    if (_initialized) return true;
    
    _initialized = true;
    Serial.println("[TaskManager] Initialized");
    return true;
}

void TaskManager::end() {
    if (_uiTaskHandle) {
        vTaskDelete(_uiTaskHandle);
        _uiTaskHandle = nullptr;
    }
    if (_bleTaskHandle) {
        vTaskDelete(_bleTaskHandle);
        _bleTaskHandle = nullptr;
    }
    if (_loggingTaskHandle) {
        vTaskDelete(_loggingTaskHandle);
        _loggingTaskHandle = nullptr;
    }
    
    _initialized = false;
}

bool TaskManager::createUITask(void (*taskFunction)(void*), void* parameter) {
    if (_uiTaskHandle) {
        Serial.println("[TaskManager] UI task already exists");
        return false;
    }
    
    BaseType_t result = xTaskCreate(
        taskFunction,
        "UITask",
        TASK_STACK_SIZE_UI,
        parameter,
        TASK_PRIORITY_UI,
        &_uiTaskHandle
    );
    
    if (result == pdPASS) {
        Serial.println("[TaskManager] UI task created");
        return true;
    } else {
        Serial.println("[TaskManager] ERROR: Failed to create UI task");
        return false;
    }
}

bool TaskManager::createBLETask(void (*taskFunction)(void*), void* parameter) {
    if (_bleTaskHandle) {
        Serial.println("[TaskManager] BLE task already exists");
        return false;
    }
    
    BaseType_t result = xTaskCreate(
        taskFunction,
        "BLETask",
        TASK_STACK_SIZE_BLE,
        parameter,
        TASK_PRIORITY_BLE,
        &_bleTaskHandle
    );
    
    if (result == pdPASS) {
        Serial.println("[TaskManager] BLE task created");
        return true;
    } else {
        Serial.println("[TaskManager] ERROR: Failed to create BLE task");
        return false;
    }
}

bool TaskManager::createLoggingTask(void (*taskFunction)(void*), void* parameter) {
    if (_loggingTaskHandle) {
        Serial.println("[TaskManager] Logging task already exists");
        return false;
    }
    
    BaseType_t result = xTaskCreate(
        taskFunction,
        "LoggingTask",
        TASK_STACK_SIZE_LOGGING,
        parameter,
        TASK_PRIORITY_LOGGING,
        &_loggingTaskHandle
    );
    
    if (result == pdPASS) {
        Serial.println("[TaskManager] Logging task created");
        return true;
    } else {
        Serial.println("[TaskManager] ERROR: Failed to create logging task");
        return false;
    }
}

void TaskManager::suspendUITask() {
    if (_uiTaskHandle) {
        vTaskSuspend(_uiTaskHandle);
    }
}

void TaskManager::resumeUITask() {
    if (_uiTaskHandle) {
        vTaskResume(_uiTaskHandle);
    }
}

void TaskManager::suspendBLETask() {
    if (_bleTaskHandle) {
        vTaskSuspend(_bleTaskHandle);
    }
}

void TaskManager::resumeBLETask() {
    if (_bleTaskHandle) {
        vTaskResume(_bleTaskHandle);
    }
}

bool TaskManager::isUITaskRunning() const {
    return _uiTaskHandle != nullptr && eTaskGetState(_uiTaskHandle) != eDeleted;
}

bool TaskManager::isBLETaskRunning() const {
    return _bleTaskHandle != nullptr && eTaskGetState(_bleTaskHandle) != eDeleted;
}

bool TaskManager::isLoggingTaskRunning() const {
    return _loggingTaskHandle != nullptr && eTaskGetState(_loggingTaskHandle) != eDeleted;
}

