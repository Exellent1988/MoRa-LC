#include "persistence_service.h"
#include <Arduino.h>

PersistenceService::PersistenceService()
    : _initialized(false) {
}

PersistenceService::~PersistenceService() {
    end();
}

bool PersistenceService::begin() {
    if (_initialized) return true;
    
    if (!_prefs.begin(NAMESPACE, false)) {
        Serial.println("[Persistence] ERROR: Failed to open NVS namespace");
        return false;
    }
    
    _initialized = true;
    Serial.println("[Persistence] Initialized");
    return true;
}

void PersistenceService::end() {
    if (_initialized) {
        _prefs.end();
        _initialized = false;
    }
}

bool PersistenceService::saveString(const char* key, const String& value) {
    if (!_initialized) return false;
    return _prefs.putString(key, value);
}

String PersistenceService::loadString(const char* key, const String& defaultValue) {
    if (!_initialized) return defaultValue;
    return _prefs.getString(key, defaultValue);
}

bool PersistenceService::saveInt(const char* key, int32_t value) {
    if (!_initialized) return false;
    return _prefs.putInt(key, value);
}

int32_t PersistenceService::loadInt(const char* key, int32_t defaultValue) {
    if (!_initialized) return defaultValue;
    return _prefs.getInt(key, defaultValue);
}

bool PersistenceService::saveFloat(const char* key, float value) {
    if (!_initialized) return false;
    return _prefs.putFloat(key, value);
}

float PersistenceService::loadFloat(const char* key, float defaultValue) {
    if (!_initialized) return defaultValue;
    return _prefs.getFloat(key, defaultValue);
}

bool PersistenceService::saveBool(const char* key, bool value) {
    if (!_initialized) return false;
    return _prefs.putBool(key, value);
}

bool PersistenceService::loadBool(const char* key, bool defaultValue) {
    if (!_initialized) return defaultValue;
    return _prefs.getBool(key, defaultValue);
}

bool PersistenceService::remove(const char* key) {
    if (!_initialized) return false;
    return _prefs.remove(key);
}

void PersistenceService::clearAll() {
    if (!_initialized) return;
    _prefs.clear();
    Serial.println("[Persistence] All data cleared");
}

