#ifndef LAP_COUNTER_H
#define LAP_COUNTER_H

#include <Arduino.h>
#include <vector>

/**
 * Lap Counter für Rundenzählung
 * 
 * Verwaltet Runden und Zeiten für Teams
 * Beide Varianten (FullBlown & UltraLight) nutzen diese Library
 */

struct LapTime {
    uint16_t lapNumber;
    uint32_t timestamp;    // millis()
    uint32_t duration;     // ms (Zeit für diese Runde)
    
    LapTime() : lapNumber(0), timestamp(0), duration(0) {}
    LapTime(uint16_t lap, uint32_t ts, uint32_t dur) 
        : lapNumber(lap), timestamp(ts), duration(dur) {}
};

struct TeamData {
    uint8_t teamId;
    String teamName;
    String beaconUUID;
    
    uint16_t lapCount;
    uint32_t lastLapTime;        // millis() der letzten Runde
    uint32_t bestLapDuration;    // ms
    uint32_t worstLapDuration;   // ms
    uint32_t totalDuration;      // ms (Summe aller Runden)
    
    std::vector<LapTime> laps;
    
    TeamData() : teamId(0), lapCount(0), lastLapTime(0), 
                 bestLapDuration(UINT32_MAX), worstLapDuration(0), totalDuration(0) {}
};

class LapCounter {
public:
    LapCounter();
    ~LapCounter();
    
    // Team-Management
    bool addTeam(uint8_t teamId, const String& teamName, const String& beaconUUID);
    bool removeTeam(uint8_t teamId);
    TeamData* getTeam(uint8_t teamId);
    TeamData* getTeamByBeacon(const String& beaconUUID);
    std::vector<TeamData*> getAllTeams();
    uint8_t getTeamCount();
    
    // Runden-Zählung
    bool recordLap(const String& beaconUUID, uint32_t timestamp = 0);
    bool recordLap(uint8_t teamId, uint32_t timestamp = 0);
    
    // Statistiken
    float getAverageLapTime(uint8_t teamId);
    uint32_t getBestLapTime(uint8_t teamId);
    uint32_t getWorstLapTime(uint8_t teamId);
    uint16_t getLapCount(uint8_t teamId);
    
    // Rangliste
    std::vector<TeamData*> getLeaderboard(bool sortByLaps = true);  // true=Runden, false=Zeit
    
    // Reset
    void reset();
    void resetTeam(uint8_t teamId);
    
    // Export
    String exportToCSV(uint8_t teamId);
    String exportAllToCSV();
    
private:
    std::vector<TeamData> teams;
    
    // Helper
    TeamData* findTeam(uint8_t teamId);
    TeamData* findTeamByBeacon(const String& beaconUUID);
    void updateStatistics(TeamData* team);
    
    static bool compareLaps(TeamData* a, TeamData* b);
    static bool compareTimes(TeamData* a, TeamData* b);
};

#endif // LAP_COUNTER_H



