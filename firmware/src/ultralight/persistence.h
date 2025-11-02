#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <Arduino.h>
#include <Preferences.h>
#include "LapCounter.h"

/**
 * Persistenz-Layer für Teams
 * 
 * Speichert Teams in ESP32 NVS (Non-Volatile Storage)
 * damit sie nach Neustart erhalten bleiben
 */

class PersistenceManager {
public:
    PersistenceManager();
    ~PersistenceManager();
    
    // Initialize
    bool begin();
    
    // Team Operations
    bool saveTeams(LapCounter& lapCounter);
    bool loadTeams(LapCounter& lapCounter);
    void clearTeams();
    
    // Config
    bool saveConfig(const String& raceName, uint32_t raceDuration);
    bool loadConfig(String& raceName, uint32_t& raceDuration);
    
    // RSSI Thresholds
    bool saveRssiThresholds(int8_t rssiNear, int8_t rssiFar);
    bool loadRssiThresholds(int8_t& rssiNear, int8_t& rssiFar);
    
    // Stats
    uint8_t getTeamCount();
    bool isInitialized();
    
private:
    Preferences preferences;
    bool initialized;
    
    // Keys
    static const char* NAMESPACE_TEAMS;
    static const char* NAMESPACE_CONFIG;
    static const char* KEY_TEAM_COUNT;
    static const char* KEY_TEAM_ID;
    static const char* KEY_TEAM_NAME;
    static const char* KEY_TEAM_BEACON;
    static const char* KEY_RACE_NAME;
    static const char* KEY_RACE_DURATION;
    static const char* KEY_RSSI_NEAR;
    static const char* KEY_RSSI_FAR;
};

#endif // PERSISTENCE_H


