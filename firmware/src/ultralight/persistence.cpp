#include "persistence.h"
#include "config.h"

const char* PersistenceManager::NAMESPACE_TEAMS = "teams";
const char* PersistenceManager::NAMESPACE_CONFIG = "config";
const char* PersistenceManager::KEY_TEAM_COUNT = "count";
const char* PersistenceManager::KEY_TEAM_ID = "id";
const char* PersistenceManager::KEY_TEAM_NAME = "name";
const char* PersistenceManager::KEY_TEAM_BEACON = "beacon";
const char* PersistenceManager::KEY_RACE_NAME = "racename";
const char* PersistenceManager::KEY_RACE_DURATION = "duration";
const char* PersistenceManager::KEY_RSSI_NEAR = "rssinear";
const char* PersistenceManager::KEY_RSSI_FAR = "rssifar";

PersistenceManager::PersistenceManager() 
    : initialized(false) {
}

PersistenceManager::~PersistenceManager() {
}

bool PersistenceManager::begin() {
    Serial.println("[Persistence] Initializing...");
    initialized = true;
    Serial.println("[Persistence] Ready");
    return true;
}

bool PersistenceManager::saveTeams(LapCounter& lapCounter) {
    if (!initialized) {
        Serial.println("[Persistence] ERROR: Not initialized");
        return false;
    }
    
    preferences.begin(NAMESPACE_TEAMS, false);  // Read-Write
    
    auto teams = lapCounter.getAllTeams();
    uint8_t teamCount = teams.size();
    
    Serial.printf("[Persistence] Saving %u teams...\n", teamCount);
    
    // Save team count
    preferences.putUChar(KEY_TEAM_COUNT, teamCount);
    
    // Save each team
    for (size_t i = 0; i < teams.size(); i++) {
        TeamData* team = teams[i];
        
        String idKey = String(KEY_TEAM_ID) + String(i);
        String nameKey = String(KEY_TEAM_NAME) + String(i);
        String beaconKey = String(KEY_TEAM_BEACON) + String(i);
        
        preferences.putUChar(idKey.c_str(), team->teamId);
        preferences.putString(nameKey.c_str(), team->teamName);
        preferences.putString(beaconKey.c_str(), team->beaconUUID);
        
        Serial.printf("[Persistence] Saved Team %u: %s\n", 
                     team->teamId, team->teamName.c_str());
    }
    
    preferences.end();
    
    Serial.println("[Persistence] Teams saved successfully");
    return true;
}

bool PersistenceManager::loadTeams(LapCounter& lapCounter) {
    if (!initialized) {
        Serial.println("[Persistence] ERROR: Not initialized");
        return false;
    }
    
    preferences.begin(NAMESPACE_TEAMS, true);  // Read-Only
    
    uint8_t teamCount = preferences.getUChar(KEY_TEAM_COUNT, 0);
    
    if (teamCount == 0) {
        Serial.println("[Persistence] No teams to load");
        preferences.end();
        return true;
    }
    
    Serial.printf("[Persistence] Loading %u teams...\n", teamCount);
    
    // Load each team
    for (uint8_t i = 0; i < teamCount; i++) {
        String idKey = String(KEY_TEAM_ID) + String(i);
        String nameKey = String(KEY_TEAM_NAME) + String(i);
        String beaconKey = String(KEY_TEAM_BEACON) + String(i);
        
        uint8_t teamId = preferences.getUChar(idKey.c_str(), 0);
        String teamName = preferences.getString(nameKey.c_str(), "");
        String beaconUUID = preferences.getString(beaconKey.c_str(), "");
        
        if (teamId > 0 && teamName.length() > 0) {
            if (lapCounter.addTeam(teamId, teamName, beaconUUID)) {
                Serial.printf("[Persistence] Loaded Team %u: %s\n", 
                             teamId, teamName.c_str());
            } else {
                Serial.printf("[Persistence] WARNING: Failed to load Team %u\n", teamId);
            }
        }
    }
    
    preferences.end();
    
    Serial.println("[Persistence] Teams loaded successfully");
    return true;
}

void PersistenceManager::clearTeams() {
    if (!initialized) return;
    
    Serial.println("[Persistence] Clearing all teams...");
    
    preferences.begin(NAMESPACE_TEAMS, false);
    preferences.clear();
    preferences.end();
    
    Serial.println("[Persistence] Teams cleared");
}

bool PersistenceManager::saveConfig(const String& raceName, uint32_t raceDuration) {
    if (!initialized) return false;
    
    preferences.begin(NAMESPACE_CONFIG, false);
    preferences.putString(KEY_RACE_NAME, raceName);
    preferences.putUInt(KEY_RACE_DURATION, raceDuration);
    preferences.end();
    
    Serial.printf("[Persistence] Config saved: %s, %lu min\n", 
                 raceName.c_str(), raceDuration);
    
    return true;
}

bool PersistenceManager::loadConfig(String& raceName, uint32_t& raceDuration) {
    if (!initialized) return false;
    
    preferences.begin(NAMESPACE_CONFIG, true);
    raceName = preferences.getString(KEY_RACE_NAME, "Rennen");
    raceDuration = preferences.getUInt(KEY_RACE_DURATION, 60);
    preferences.end();
    
    Serial.printf("[Persistence] Config loaded: %s, %lu min\n", 
                 raceName.c_str(), raceDuration);
    
    return true;
}

uint8_t PersistenceManager::getTeamCount() {
    if (!initialized) return 0;
    
    preferences.begin(NAMESPACE_TEAMS, true);
    uint8_t count = preferences.getUChar(KEY_TEAM_COUNT, 0);
    preferences.end();
    
    return count;
}

bool PersistenceManager::saveRssiThresholds(int8_t rssiNear, int8_t rssiFar) {
    if (!initialized) return false;
    
    preferences.begin(NAMESPACE_CONFIG, false);
    preferences.putChar(KEY_RSSI_NEAR, rssiNear);
    preferences.putChar(KEY_RSSI_FAR, rssiFar);
    preferences.end();
    
    Serial.printf("[Persistence] RSSI thresholds saved: NEAR=%d, FAR=%d\n", 
                 rssiNear, rssiFar);
    return true;
}

bool PersistenceManager::loadRssiThresholds(int8_t& rssiNear, int8_t& rssiFar) {
    if (!initialized) return false;
    
    preferences.begin(NAMESPACE_CONFIG, true);
    rssiNear = preferences.getChar(KEY_RSSI_NEAR, DEFAULT_LAP_RSSI_NEAR);
    rssiFar = preferences.getChar(KEY_RSSI_FAR, DEFAULT_LAP_RSSI_FAR);
    preferences.end();
    
    Serial.printf("[Persistence] RSSI thresholds loaded: NEAR=%d, FAR=%d\n", 
                 rssiNear, rssiFar);
    return true;
}

bool PersistenceManager::isInitialized() {
    return initialized;
}


