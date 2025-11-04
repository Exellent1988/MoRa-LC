#ifndef PERSISTENCE_SERVICE_H
#define PERSISTENCE_SERVICE_H

#include <Arduino.h>
#include <Preferences.h>

/**
 * Persistence Service
 * NVS-based storage for configuration and data
 */

class PersistenceService {
public:
    PersistenceService();
    ~PersistenceService();
    
    // Initialization
    bool begin();
    void end();
    
    // String operations
    bool saveString(const char* key, const String& value);
    String loadString(const char* key, const String& defaultValue = "");
    
    // Integer operations
    bool saveInt(const char* key, int32_t value);
    int32_t loadInt(const char* key, int32_t defaultValue = 0);
    
    // Float operations
    bool saveFloat(const char* key, float value);
    float loadFloat(const char* key, float defaultValue = 0.0f);
    
    // Boolean operations
    bool saveBool(const char* key, bool value);
    bool loadBool(const char* key, bool defaultValue = false);
    
    // Clear operations
    bool remove(const char* key);
    void clearAll();
    
    // Status
    bool isReady() const { return _initialized; }
    
private:
    Preferences _prefs;
    bool _initialized;
    static constexpr const char* NAMESPACE = "mora_lc";
};

#endif // PERSISTENCE_SERVICE_H

