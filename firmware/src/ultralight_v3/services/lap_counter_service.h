#ifndef LAP_COUNTER_SERVICE_H
#define LAP_COUNTER_SERVICE_H

#include <Arduino.h>
#include "../core/config.h"
#include "../hardware/ble.h"
#include "../services/beacon_service.h"
#include <vector>
#include <map>

// Forward declaration
class PersistenceService;

/**
 * Lap Counter Service
 * Handles lap counting logic, team management, and race state
 */

struct TeamData {
    uint8_t teamId;
    String teamName;
    String beaconUUID;  // MAC address or UUID
    uint32_t lapCount;
    uint32_t lastLapTime;  // millis()
    uint32_t bestLapTime;  // milliseconds
    bool isActive;
    
    TeamData() : teamId(0), lapCount(0), lastLapTime(0), bestLapTime(0), isActive(false) {}
};

typedef std::function<void(const TeamData& team, int8_t rssi)> LapCounterCallback;

/**
 * Lap Counter Service
 * Manages teams, lap counting, and race statistics
 */
class LapCounterService {
public:
    LapCounterService();
    ~LapCounterService();
    
    // Initialization
    bool begin(BeaconService* beaconService);
    void end();
    
    // Persistence
    bool saveTeams(PersistenceService* persistence);
    bool loadTeams(PersistenceService* persistence);
    
    // Team management
    bool addTeam(const String& name);
    bool removeTeam(uint8_t teamId);
    bool updateTeam(uint8_t teamId, const String& name);
    bool assignBeacon(uint8_t teamId, const String& beaconUUID);
    TeamData* getTeam(uint8_t teamId);
    std::vector<TeamData*> getAllTeams();
    uint8_t getTeamCount() const { return _teams.size(); }
    
    // Race management
    void startRace();
    void stopRace();
    void resetRace();
    bool isRaceActive() const { return _raceActive; }
    
    // Lap counting
    void update();  // Call in loop for lap detection
    uint32_t getLapCount(uint8_t teamId) const;
    uint32_t getBestLapTime(uint8_t teamId) const;
    
    // Callbacks
    void setLapCallback(LapCounterCallback callback) { _lapCallback = callback; }
    
    // Statistics
    std::vector<TeamData*> getLeaderboard();  // Sorted by lap count
    
private:
    BeaconService* _beaconService;
    std::map<uint8_t, TeamData> _teams;
    uint8_t _nextTeamId;
    bool _raceActive;
    uint32_t _raceStartTime;
    
    // Lap detection state
    std::map<String, bool> _beaconPresence;  // Track presence per beacon
    std::map<String, uint32_t> _lastLapTime;  // Prevent duplicate lap detection
    
    LapCounterCallback _lapCallback;
    
    // Helper methods
    void handleBeaconUpdate(const BeaconInfo& beacon);
    bool detectLap(const String& beaconUUID, const BeaconInfo& beacon);
};

#endif // LAP_COUNTER_SERVICE_H

