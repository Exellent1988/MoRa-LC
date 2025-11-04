#include "lap_counter_service.h"
#include <algorithm>

LapCounterService::LapCounterService()
    : _beaconService(nullptr)
    , _nextTeamId(1)
    , _raceActive(false)
    , _raceStartTime(0) {
}

LapCounterService::~LapCounterService() {
    end();
}

bool LapCounterService::begin(BeaconService* beaconService) {
    if (!beaconService) {
        Serial.println("[LapCounter] ERROR: BeaconService is null");
        return false;
    }
    
    _beaconService = beaconService;
    
    // Set up beacon callback
    _beaconService->setBeaconCallback([this](const BeaconInfo& beacon) {
        this->handleBeaconUpdate(beacon);
    });
    
    Serial.println("[LapCounter] Initialized");
    return true;
}

void LapCounterService::end() {
    _beaconService = nullptr;
    _teams.clear();
    _beaconPresence.clear();
    _lastLapTime.clear();
}

bool LapCounterService::addTeam(const String& name) {
    if (_teams.size() >= MAX_TEAMS) {
        Serial.println("[LapCounter] ERROR: Max teams reached");
        return false;
    }
    
    TeamData team;
    team.teamId = _nextTeamId++;
    team.teamName = name;
    team.isActive = true;
    
    _teams[team.teamId] = team;
    
    Serial.printf("[LapCounter] Team added: ID=%u, Name=%s\n", team.teamId, name.c_str());
    return true;
}

bool LapCounterService::removeTeam(uint8_t teamId) {
    auto it = _teams.find(teamId);
    if (it == _teams.end()) {
        return false;
    }
    
    _teams.erase(it);
    Serial.printf("[LapCounter] Team removed: ID=%u\n", teamId);
    return true;
}

bool LapCounterService::updateTeam(uint8_t teamId, const String& name) {
    auto it = _teams.find(teamId);
    if (it == _teams.end()) {
        return false;
    }
    
    it->second.teamName = name;
    Serial.printf("[LapCounter] Team updated: ID=%u, Name=%s\n", teamId, name.c_str());
    return true;
}

bool LapCounterService::assignBeacon(uint8_t teamId, const String& beaconUUID) {
    auto it = _teams.find(teamId);
    if (it == _teams.end()) {
        return false;
    }
    
    it->second.beaconUUID = beaconUUID;
    Serial.printf("[LapCounter] Beacon assigned: Team=%u, Beacon=%s\n", teamId, beaconUUID.c_str());
    return true;
}

TeamData* LapCounterService::getTeam(uint8_t teamId) {
    auto it = _teams.find(teamId);
    if (it == _teams.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<TeamData*> LapCounterService::getAllTeams() {
    std::vector<TeamData*> result;
    for (auto& pair : _teams) {
        result.push_back(&pair.second);
    }
    return result;
}

void LapCounterService::startRace() {
    _raceActive = true;
    _raceStartTime = millis();
    
    // Reset lap counts
    for (auto& pair : _teams) {
        pair.second.lapCount = 0;
        pair.second.lastLapTime = 0;
        pair.second.bestLapTime = 0;
    }
    
    // Clear presence state
    _beaconPresence.clear();
    _lastLapTime.clear();
    
    Serial.println("[LapCounter] Race started");
}

void LapCounterService::stopRace() {
    _raceActive = false;
    Serial.println("[LapCounter] Race stopped");
}

void LapCounterService::resetRace() {
    stopRace();
    _teams.clear();
    _nextTeamId = 1;
    Serial.println("[LapCounter] Race reset");
}

void LapCounterService::update() {
    // Lap detection is handled via beacon callbacks
    // This method is kept for future use (e.g., periodic updates)
}

uint32_t LapCounterService::getLapCount(uint8_t teamId) const {
    auto it = _teams.find(teamId);
    if (it == _teams.end()) {
        return 0;
    }
    return it->second.lapCount;
}

uint32_t LapCounterService::getBestLapTime(uint8_t teamId) const {
    auto it = _teams.find(teamId);
    if (it == _teams.end()) {
        return 0;
    }
    return it->second.bestLapTime;
}

void LapCounterService::handleBeaconUpdate(const BeaconInfo& beacon) {
    if (!_raceActive) return;
    
    // Find team with matching beacon
    for (auto& pair : _teams) {
        if (pair.second.beaconUUID == beacon.macAddress || 
            pair.second.beaconUUID == beacon.uuid) {
            // Check for lap detection
            if (detectLap(beacon.macAddress, beacon)) {
                pair.second.lapCount++;
                uint32_t now = millis();
                pair.second.lastLapTime = now;
                
                // Calculate lap time (if this is not the first lap)
                if (pair.second.lapCount > 1) {
                    uint32_t lapTime = now - _lastLapTime[beacon.macAddress];
                    if (pair.second.bestLapTime == 0 || lapTime < pair.second.bestLapTime) {
                        pair.second.bestLapTime = lapTime;
                    }
                }
                
                _lastLapTime[beacon.macAddress] = now;
                
                // Notify callback
                if (_lapCallback) {
                    _lapCallback(pair.second.teamId, pair.second.lapCount);
                }
                
                Serial.printf("[LapCounter] Lap detected: Team=%u (%s), Laps=%u\n",
                            pair.second.teamId, pair.second.teamName.c_str(), pair.second.lapCount);
            }
            break;
        }
    }
}

bool LapCounterService::detectLap(const String& beaconUUID, const BeaconInfo& beacon) {
    // Simple lap detection: beacon crosses threshold (near -> far -> near)
    bool wasPresent = _beaconPresence[beaconUUID];
    bool isPresent = beacon.avgRssi >= DEFAULT_LAP_RSSI_NEAR;
    
    _beaconPresence[beaconUUID] = isPresent;
    
    // Detect crossing: was far, now near (and enough time passed)
    if (!wasPresent && isPresent) {
        uint32_t now = millis();
        auto lastTimeIt = _lastLapTime.find(beaconUUID);
        
        if (lastTimeIt == _lastLapTime.end() || 
            (now - lastTimeIt->second) >= MIN_LAP_TIME) {
            return true;
        }
    }
    
    return false;
}

std::vector<TeamData*> LapCounterService::getLeaderboard() {
    std::vector<TeamData*> result = getAllTeams();
    
    // Sort by lap count (descending), then by best lap time (ascending)
    std::sort(result.begin(), result.end(), [](TeamData* a, TeamData* b) {
        if (a->lapCount != b->lapCount) {
            return a->lapCount > b->lapCount;
        }
        if (a->bestLapTime != 0 && b->bestLapTime != 0) {
            return a->bestLapTime < b->bestLapTime;
        }
        return a->bestLapTime != 0; // Teams with lap times come first
    });
    
    return result;
}

