#include "LapCounter.h"
#include <algorithm>

#ifndef MAX_TEAMS
#define MAX_TEAMS 20  // Default max teams if not defined in config
#endif

LapCounter::LapCounter() {
}

LapCounter::~LapCounter() {
}

bool LapCounter::addTeam(uint8_t teamId, const String& teamName, const String& beaconUUID) {
    // Prüfe ob Team bereits existiert
    if (findTeam(teamId) != nullptr) {
        Serial.printf("[LapCounter] Team %u already exists\n", teamId);
        return false;
    }
    
    // Prüfe ob Beacon bereits zugeordnet (NUR wenn nicht leer!)
    if (beaconUUID.length() > 0 && findTeamByBeacon(beaconUUID) != nullptr) {
        Serial.printf("[LapCounter] Beacon %s already assigned\n", beaconUUID.c_str());
        return false;
    }
    
    TeamData team;
    team.teamId = teamId;
    team.teamName = teamName;
    team.beaconUUID = beaconUUID;
    
    teams.push_back(team);
    
    Serial.printf("[LapCounter] Team added: ID=%u, Name=%s, Beacon=%s\n",
                 teamId, teamName.c_str(), beaconUUID.c_str());
    
    return true;
}

bool LapCounter::removeTeam(uint8_t teamId) {
    auto it = teams.begin();
    while (it != teams.end()) {
        if (it->teamId == teamId) {
            Serial.printf("[LapCounter] Team removed: ID=%u, Name=%s\n",
                         it->teamId, it->teamName.c_str());
            teams.erase(it);
            return true;
        }
        ++it;
    }
    return false;
}

TeamData* LapCounter::getTeam(uint8_t teamId) {
    return findTeam(teamId);
}

TeamData* LapCounter::getTeamByBeacon(const String& beaconUUID) {
    return findTeamByBeacon(beaconUUID);
}

std::vector<TeamData*> LapCounter::getAllTeams() {
    std::vector<TeamData*> result;
    for (auto& team : teams) {
        result.push_back(&team);
    }
    return result;
}

uint8_t LapCounter::getTeamCount() {
    return teams.size();
}

uint8_t LapCounter::getNextFreeTeamId() {
    // Find the next free ID starting from 1
    for (uint8_t id = 1; id <= MAX_TEAMS; id++) {
        if (findTeam(id) == nullptr) {
            return id;
        }
    }
    return 0;  // No free ID found
}

bool LapCounter::recordLap(const String& beaconUUID, uint32_t timestamp) {
    TeamData* team = findTeamByBeacon(beaconUUID);
    if (!team) {
        Serial.printf("[LapCounter] No team found for beacon: %s\n", beaconUUID.c_str());
        return false;
    }
    
    return recordLap(team->teamId, timestamp);
}

bool LapCounter::recordLap(uint8_t teamId, uint32_t timestamp) {
    TeamData* team = findTeam(teamId);
    if (!team) {
        Serial.printf("[LapCounter] Team %u not found\n", teamId);
        return false;
    }
    
    // Timestamp verwenden oder aktuell
    if (timestamp == 0) {
        timestamp = millis();
    }
    
    // Erste Runde: Nur Startzeit speichern
    if (team->lapCount == 0) {
        team->lastLapTime = timestamp;
        team->lapCount = 1;
        
        Serial.printf("[LapCounter] Team %u (%s): Started\n",
                     team->teamId, team->teamName.c_str());
        return true;
    }
    
    // Runden-Dauer berechnen
    uint32_t duration = timestamp - team->lastLapTime;
    
    // Plausibilitätsprüfung: Mindestens 5 Sekunden pro Runde
    if (duration < 5000) {
        Serial.printf("[LapCounter] Team %u: Lap too fast (%u ms < 5000 ms), ignored\n",
                     team->teamId, duration);
        return false;
    }
    
    // Lap Time speichern
    LapTime lap(team->lapCount, timestamp, duration);
    team->laps.push_back(lap);
    
    // Update Team-Daten
    team->lapCount++;
    team->lastLapTime = timestamp;
    team->totalDuration += duration;
    
    // Statistiken aktualisieren
    updateStatistics(team);
    
    Serial.printf("[LapCounter] Team %u (%s): Lap %u - Time: %u.%03u s (Best: %u.%03u s)\n",
                 team->teamId, team->teamName.c_str(), team->lapCount - 1,
                 duration / 1000, duration % 1000,
                 team->bestLapDuration / 1000, team->bestLapDuration % 1000);
    
    return true;
}

float LapCounter::getAverageLapTime(uint8_t teamId) {
    TeamData* team = findTeam(teamId);
    if (!team || team->laps.empty()) {
        return 0.0f;
    }
    
    return (float)team->totalDuration / (float)team->laps.size();
}

uint32_t LapCounter::getBestLapTime(uint8_t teamId) {
    TeamData* team = findTeam(teamId);
    if (!team) {
        return 0;
    }
    return team->bestLapDuration;
}

uint32_t LapCounter::getWorstLapTime(uint8_t teamId) {
    TeamData* team = findTeam(teamId);
    if (!team) {
        return 0;
    }
    return team->worstLapDuration;
}

uint16_t LapCounter::getLapCount(uint8_t teamId) {
    TeamData* team = findTeam(teamId);
    if (!team) {
        return 0;
    }
    return team->lapCount - 1;  // -1 weil erste "Runde" nur Start ist
}

std::vector<TeamData*> LapCounter::getLeaderboard(bool sortByLaps) {
    std::vector<TeamData*> leaderboard;
    for (auto& team : teams) {
        leaderboard.push_back(&team);
    }
    
    if (sortByLaps) {
        std::sort(leaderboard.begin(), leaderboard.end(), compareLaps);
    } else {
        std::sort(leaderboard.begin(), leaderboard.end(), compareTimes);
    }
    
    return leaderboard;
}

void LapCounter::reset() {
    for (auto& team : teams) {
        resetTeam(team.teamId);
    }
    Serial.println("[LapCounter] All teams reset");
}

void LapCounter::resetTeam(uint8_t teamId) {
    TeamData* team = findTeam(teamId);
    if (!team) {
        return;
    }
    
    team->lapCount = 0;
    team->lastLapTime = 0;
    team->bestLapDuration = UINT32_MAX;
    team->worstLapDuration = 0;
    team->totalDuration = 0;
    team->laps.clear();
    
    Serial.printf("[LapCounter] Team %u reset\n", teamId);
}

String LapCounter::exportToCSV(uint8_t teamId) {
    TeamData* team = findTeam(teamId);
    if (!team) {
        return "";
    }
    
    String csv = "Lap,Timestamp,Duration (ms)\n";
    
    for (auto& lap : team->laps) {
        csv += String(lap.lapNumber) + ",";
        csv += String(lap.timestamp) + ",";
        csv += String(lap.duration) + "\n";
    }
    
    return csv;
}

String LapCounter::exportAllToCSV() {
    String csv = "Team ID,Team Name,Lap,Timestamp,Duration (ms)\n";
    
    for (auto& team : teams) {
        for (auto& lap : team.laps) {
            csv += String(team.teamId) + ",";
            csv += team.teamName + ",";
            csv += String(lap.lapNumber) + ",";
            csv += String(lap.timestamp) + ",";
            csv += String(lap.duration) + "\n";
        }
    }
    
    return csv;
}

// ============================================================
// Private Helper
// ============================================================

TeamData* LapCounter::findTeam(uint8_t teamId) {
    for (auto& team : teams) {
        if (team.teamId == teamId) {
            return &team;
        }
    }
    return nullptr;
}

TeamData* LapCounter::findTeamByBeacon(const String& beaconUUID) {
    // Leere Beacon-Strings niemals matchen (viele Teams können keinen Beacon haben)
    if (beaconUUID.length() == 0) {
        return nullptr;
    }
    
    for (auto& team : teams) {
        if (team.beaconUUID == beaconUUID) {
            return &team;
        }
    }
    return nullptr;
}

void LapCounter::updateStatistics(TeamData* team) {
    if (team->laps.empty()) {
        return;
    }
    
    // Beste und schlechteste Rundenzeit
    for (auto& lap : team->laps) {
        if (lap.duration < team->bestLapDuration) {
            team->bestLapDuration = lap.duration;
        }
        if (lap.duration > team->worstLapDuration) {
            team->worstLapDuration = lap.duration;
        }
    }
}

bool LapCounter::compareLaps(TeamData* a, TeamData* b) {
    // Mehr Runden = besser
    if (a->lapCount != b->lapCount) {
        return a->lapCount > b->lapCount;
    }
    // Bei gleicher Rundenzahl: Schnellere Zeit = besser
    return a->totalDuration < b->totalDuration;
}

bool LapCounter::compareTimes(TeamData* a, TeamData* b) {
    // Schnellere beste Rundenzeit = besser
    return a->bestLapDuration < b->bestLapDuration;
}



