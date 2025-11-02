#include "ui_screens.h"
#include "persistence.h"
#include "DataLogger.h"
#include <algorithm>  // For std::sort
#include <map>

extern bool raceRunning;
extern uint32_t raceStartTime;
extern uint32_t raceDuration;
extern String currentRaceName;
extern DataLogger dataLogger;
extern PersistenceManager persistence;
extern std::map<uint8_t, bool> beaconPresence;

// ============================================================
// Main Touch Handler
// ============================================================

void handleTouch(uint16_t x, uint16_t y) {
    switch (uiState.currentScreen) {
        case SCREEN_HOME:
            handleHomeTouch(x, y);
            break;
        case SCREEN_TEAMS:
            handleTeamsTouch(x, y);
            break;
        case SCREEN_TEAM_EDIT:
            handleTeamEditTouch(x, y);
            break;
        case SCREEN_TEAM_BEACON_ASSIGN:
            handleBeaconAssignTouch(x, y);
            break;
        case SCREEN_BEACON_LIST:
            handleBeaconListTouch(x, y);
            break;
        case SCREEN_RACE_SETUP:
            handleRaceSetupTouch(x, y);
            break;
        case SCREEN_RACE_RUNNING:
            handleRaceRunningTouch(x, y);
            break;
        case SCREEN_RACE_PAUSED:
            handleRacePausedTouch(x, y);
            break;
        case SCREEN_RACE_RESULTS:
            handleRaceResultsTouch(x, y);
            break;
        case SCREEN_SETTINGS:
            // Back button
            if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
                uiState.currentScreen = SCREEN_HOME;
                uiState.needsRedraw = true;
            }
            break;
        default:
            break;
    }
}

// ============================================================
// Screen-Specific Touch Handlers
// ============================================================

void handleHomeTouch(uint16_t x, uint16_t y) {
    int btnY = HEADER_HEIGHT + 20;
    int btnWidth = SCREEN_WIDTH - 2 * BUTTON_MARGIN;
    
    // Neues Rennen
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnWidth, BUTTON_HEIGHT)) {
        uiState.currentScreen = SCREEN_RACE_SETUP;
        uiState.needsRedraw = true;
        return;
    }
    btnY += BUTTON_HEIGHT + BUTTON_MARGIN;
    
    // Teams verwalten
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnWidth, BUTTON_HEIGHT)) {
        uiState.currentScreen = SCREEN_TEAMS;
        uiState.needsRedraw = true;
        return;
    }
    btnY += BUTTON_HEIGHT + BUTTON_MARGIN;
    
    // Ergebnisse
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnWidth, BUTTON_HEIGHT)) {
        if (lapCounter.getTeamCount() > 0) {
            uiState.currentScreen = SCREEN_RACE_RESULTS;
            uiState.needsRedraw = true;
        } else {
            showMessage("Info", "Keine Ergebnisse vorhanden", TFT_YELLOW);
        }
        return;
    }
    btnY += BUTTON_HEIGHT + BUTTON_MARGIN;
    
    // Einstellungen
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnWidth, BUTTON_HEIGHT)) {
        uiState.currentScreen = SCREEN_SETTINGS;
        uiState.needsRedraw = true;
        return;
    }
}

void handleTeamsTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
        uiState.currentScreen = SCREEN_HOME;
        uiState.needsRedraw = true;
        return;
    }
    
    // Team items
    auto teams = lapCounter.getAllTeams();
    int itemY = HEADER_HEIGHT + 10;
    int itemNum = 0;
    
    for (auto* team : teams) {
        if (itemNum >= 4) break;  // Max 4 visible
        
        if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 20, LIST_ITEM_HEIGHT)) {
            // Edit this team
            uiState.editingTeamId = team->teamId;
            uiState.editingTeamName = team->teamName;
            uiState.currentScreen = SCREEN_TEAM_EDIT;
            uiState.needsRedraw = true;
            return;
        }
        
        itemY += LIST_ITEM_HEIGHT + 5;
        itemNum++;
    }
    
    // Add new team button
    int btnY = SCREEN_HEIGHT - 60;
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, BUTTON_HEIGHT)) {
        if (lapCounter.getTeamCount() >= MAX_TEAMS) {
            showMessage("Fehler", "Maximale Teams erreicht", TFT_RED);
        } else {
            // Create new team with default name
            uint8_t newId = lapCounter.getTeamCount() + 1;
            String newName = "Team " + String(newId);
            
            if (lapCounter.addTeam(newId, newName, "")) {
                // Save to NVS
                if (persistence.isInitialized()) {
                    persistence.saveTeams(lapCounter);
                }
                
                uiState.editingTeamId = newId;
                uiState.editingTeamName = newName;
                uiState.currentScreen = SCREEN_TEAM_EDIT;
                uiState.needsRedraw = true;
            } else {
                showMessage("Fehler", "Team konnte nicht erstellt werden", TFT_RED);
            }
        }
    }
}

void handleTeamEditTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
        uiState.currentScreen = SCREEN_TEAMS;
        uiState.needsRedraw = true;
        return;
    }
    
    int btnY = HEADER_HEIGHT + 20;
    
    // Team Name edit
    if (isTouchInRect(x, y, 10, btnY + 25, SCREEN_WIDTH - 20, 40)) {
        String newName = inputText("Team-Name", uiState.editingTeamName);
        if (newName.length() > 0) {
            uiState.editingTeamName = newName;
            uiState.needsRedraw = true;
        }
        return;
    }
    
    btnY += 80;
    
    // Beacon assignment
    if (isTouchInRect(x, y, 10, btnY + 25, SCREEN_WIDTH - 20, 40)) {
        uiState.previousScreen = SCREEN_TEAM_EDIT;
        uiState.currentScreen = SCREEN_TEAM_BEACON_ASSIGN;
        uiState.needsRedraw = true;
        
        // Start BLE scanning
        if (!bleScanner.isScanning()) {
            bleScanner.startScan();
        }
        return;
    }
    
    btnY += 80;
    
    // Save button
    if (isTouchInRect(x, y, 10, btnY, (SCREEN_WIDTH - 30) / 2, BUTTON_HEIGHT)) {
        TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
        if (team) {
            team->teamName = uiState.editingTeamName;
            
            // Save to NVS
            if (persistence.isInitialized()) {
                persistence.saveTeams(lapCounter);
            }
            
            showMessage("Gespeichert", team->teamName, COLOR_SECONDARY);
            uiState.currentScreen = SCREEN_TEAMS;
            uiState.needsRedraw = true;
        }
        return;
    }
    
    // Delete button
    if (isTouchInRect(x, y, (SCREEN_WIDTH + 10) / 2, btnY, (SCREEN_WIDTH - 30) / 2, BUTTON_HEIGHT)) {
        if (lapCounter.removeTeam(uiState.editingTeamId)) {
            // Save to NVS
            if (persistence.isInitialized()) {
                persistence.saveTeams(lapCounter);
            }
            
            showMessage("Geloscht", "Team wurde entfernt", COLOR_DANGER);
            uiState.currentScreen = SCREEN_TEAMS;
            uiState.needsRedraw = true;
        }
        return;
    }
}

void handleBeaconAssignTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
        bleScanner.stopScan();
        uiState.currentScreen = uiState.previousScreen;
        uiState.needsRedraw = true;
        return;
    }
    
    // Assign nearest beacon button (if visible and close enough)
    BeaconData* nearest = bleScanner.getNearestBeacon();
    if (nearest) {
        float dist = BLEScanner::rssiToDistance(nearest->rssi, nearest->txPower);
        
        if (dist < 1.0) {
            int btnY = HEADER_HEIGHT + 107;  // Position der neuen Buttons
            int btnW = (SCREEN_WIDTH - 30) / 2;
            
            // "Zuordnen" Button (links)
            if (isTouchInRect(x, y, 10, btnY, btnW, BUTTON_HEIGHT)) {
                // Assign beacon to current team
                TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
                if (team) {
                    team->beaconUUID = nearest->uuid;
                    
                    // Save to NVS
                    if (persistence.isInitialized()) {
                        persistence.saveTeams(lapCounter);
                    }
                    
                    showMessage("Zugeordnet", "Beacon erfolgreich zugeordnet", COLOR_SECONDARY);
                    bleScanner.stopScan();
                    uiState.currentScreen = SCREEN_TEAM_EDIT;
                    uiState.needsRedraw = true;
                }
                return;
            }
            
            // "Liste" Button (rechts)
            if (isTouchInRect(x, y, 20 + btnW, btnY, btnW, BUTTON_HEIGHT)) {
                uiState.currentScreen = SCREEN_BEACON_LIST;
                uiState.needsRedraw = true;
                return;
            }
        }
    }
    
    // "Beacon-Liste anzeigen" Button (immer unten)
    int btnY = SCREEN_HEIGHT - 55;
    if (isTouchInRect(x, y, 10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT)) {
        uiState.currentScreen = SCREEN_BEACON_LIST;
        uiState.needsRedraw = true;
        return;
    }
}

void handleBeaconListTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
        uiState.currentScreen = SCREEN_TEAM_BEACON_ASSIGN;
        uiState.needsRedraw = true;
        return;
    }
    
    auto beacons = bleScanner.getBeacons();
    if (beacons.empty()) return;
    
    // *** WICHTIG: Gleiche Sortierung wie in drawBeaconListScreen! ***
    std::sort(beacons.begin(), beacons.end(), [](const BeaconData& a, const BeaconData& b) {
        return a.rssi > b.rssi;  // Höherer RSSI = stärker = näher
    });
    
    int y_start = HEADER_HEIGHT + 30;
    int itemHeight = 34;  // Kompakter: 42 -> 34
    
    int index = 0;
    for (auto& beacon : beacons) {
        if (index >= 8) break;  // Max 8 (gleich wie Drawing)
        
        int itemY = y_start + (index * itemHeight);
        
        if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 20, 32)) {  // Kompakter: 38 -> 32
            Serial.printf("[Touch] Beacon #%d angeklickt: %s (RSSI=%d) at y=%d\n", 
                         index, beacon.uuid.c_str(), beacon.rssi, itemY);
            
            float dist = BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower);
            
            if (dist < 1.0) {
                // Assign this beacon to the current team (using editingTeamId)
                TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
                
                if (team) {
                    team->beaconUUID = beacon.uuid;
                    
                    // Save to NVS
                    if (persistence.isInitialized()) {
                        persistence.saveTeams(lapCounter);
                    }
                    
                    Serial.printf("[Team %u] Beacon zugeordnet: %s\n", uiState.editingTeamId, beacon.uuid.c_str());
                    
                    showMessage("Zugeordnet", "Beacon erfolgreich zugeordnet", COLOR_SECONDARY);
                    bleScanner.stopScan();
                    uiState.currentScreen = SCREEN_TEAM_EDIT;
                    uiState.needsRedraw = true;
                    return;
                }
            } else {
                showMessage("Hinweis", "Beacon zu weit entfernt!\nBitte naher halten (<1m)", TFT_ORANGE);
            }
            return;
        }
        
        index++;
    }
}

void handleRaceSetupTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
        uiState.currentScreen = SCREEN_HOME;
        uiState.needsRedraw = true;
        return;
    }
    
    int btnY = HEADER_HEIGHT + 10;
    
    // === Race Name Input ===
    if (isTouchInRect(x, y, 10, btnY + 12, SCREEN_WIDTH - 20, 32)) {
        uiState.raceName = inputText("Rennen-Name", uiState.raceName);
        uiState.needsRedraw = true;
        return;
    }
    
    btnY += 50 + 12;  // Race name + label
    
    // === Duration Controls ===
    int btnSize = 38;
    
    // "-5" Button
    if (isTouchInRect(x, y, 10, btnY, 45, btnSize)) {
        if (uiState.raceDuration >= 5) {
            uiState.raceDuration -= 5;
        } else {
            uiState.raceDuration = 1;  // Minimum 1 Minute
        }
        uiState.needsRedraw = true;
        return;
    }
    
    // "+5" Button
    if (isTouchInRect(x, y, 185, btnY, 45, btnSize)) {
        if (uiState.raceDuration <= 175) {
            uiState.raceDuration += 5;
        } else {
            uiState.raceDuration = 180;  // Maximum 180 Minuten
        }
        uiState.needsRedraw = true;
        return;
    }
    
    // "+15" Button
    if (isTouchInRect(x, y, 235, btnY, 45, btnSize)) {
        if (uiState.raceDuration <= 165) {
            uiState.raceDuration += 15;
        } else {
            uiState.raceDuration = 180;  // Maximum 180 Minuten
        }
        uiState.needsRedraw = true;
        return;
    }
    
    // Zeit-Anzeige klicken für manuellen Input
    if (isTouchInRect(x, y, 60, btnY, 120, btnSize)) {
        uiState.raceDuration = inputNumber("Dauer (Minuten)", uiState.raceDuration, 1, 180);
        uiState.needsRedraw = true;
        return;
    }
    
    btnY += 48 + 12;  // Duration + label
    
    // === Team Checkboxes ===
    int teamCount = lapCounter.getTeamCount();
    if (teamCount > 0) {
        btnY += 12;  // Team label
        
        for (uint8_t i = 1; i <= MAX_TEAMS && i <= teamCount; i++) {
            if (btnY > SCREEN_HEIGHT - 60) break;
            
            if (isTouchInRect(x, y, 10, btnY, SCREEN_WIDTH - 20, 20)) {
                // Toggle team selection
                uiState.selectedTeams[i-1] = !uiState.selectedTeams[i-1];
                uiState.needsRedraw = true;
                return;
            }
            
            btnY += 23;
        }
    }
    
    // === Start Button ===
    btnY = SCREEN_HEIGHT - 50;
    if (isTouchInRect(x, y, 10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT)) {
        // Check if any teams exist
        if (lapCounter.getTeamCount() == 0) {
            showMessage("Fehler", "Keine Teams vorhanden!", TFT_RED);
            return;
        }
        
        // Start race
        currentRaceName = uiState.raceName;
        raceDuration = uiState.raceDuration * 60 * 1000;  // Convert to ms
        raceStartTime = millis();
        raceRunning = true;
        
        // Save race config
        if (persistence.isInitialized()) {
            persistence.saveConfig(currentRaceName, uiState.raceDuration);
        }
        
        // Start data logging
        if (dataLogger.isReady()) {
            dataLogger.startNewRace(currentRaceName);
        }
        
        // Start BLE scanning
        if (!bleScanner.isScanning()) {
            bleScanner.startScan();
        }
        
        // Reset all teams
        lapCounter.reset();
        
        // Reset beacon presence tracking
        beaconPresence.clear();
        Serial.println("[Race] Beacon presence tracking reset");
        
        uiState.currentScreen = SCREEN_RACE_RUNNING;
        uiState.needsRedraw = true;
        
        Serial.println("\n=== RACE STARTED ===");
        Serial.printf("Name: %s\n", currentRaceName.c_str());
        Serial.printf("Duration: %lu minutes\n", uiState.raceDuration);
        Serial.printf("Teams: %u\n", lapCounter.getTeamCount());
    }
}

void handleRaceRunningTouch(uint16_t x, uint16_t y) {
    int btnY = SCREEN_HEIGHT - 50;
    
    // Pause button
    if (isTouchInRect(x, y, 10, btnY, 100, 40)) {
        raceRunning = false;
        uiState.currentScreen = SCREEN_RACE_PAUSED;
        uiState.needsRedraw = true;
        Serial.println("\n=== RACE PAUSED ===");
        return;
    }
    
    // Stop button
    if (isTouchInRect(x, y, 120, btnY, 100, 40)) {
        raceRunning = false;
        uiState.currentScreen = SCREEN_RACE_RESULTS;
        uiState.needsRedraw = true;
        
        // Finish race
        if (dataLogger.isReady()) {
            dataLogger.finishRace();
        }
        bleScanner.stopScan();
        
        Serial.println("\n=== RACE STOPPED ===");
        return;
    }
}

void handleRacePausedTouch(uint16_t x, uint16_t y) {
    int btnY = SCREEN_HEIGHT - 110;
    
    // Continue button
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, BUTTON_HEIGHT)) {
        raceRunning = true;
        uiState.currentScreen = SCREEN_RACE_RUNNING;
        uiState.needsRedraw = true;
        
        if (!bleScanner.isScanning()) {
            bleScanner.startScan();
        }
        
        Serial.println("\n=== RACE RESUMED ===");
        return;
    }
    
    btnY += BUTTON_HEIGHT + 10;
    
    // End race button
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, BUTTON_HEIGHT)) {
        raceRunning = false;
        uiState.currentScreen = SCREEN_RACE_RESULTS;
        uiState.needsRedraw = true;
        
        if (dataLogger.isReady()) {
            dataLogger.finishRace();
        }
        bleScanner.stopScan();
        
        Serial.println("\n=== RACE ENDED ===");
        return;
    }
}

void handleRaceResultsTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
        uiState.currentScreen = SCREEN_HOME;
        uiState.needsRedraw = true;
        return;
    }
    
    // SD Card info button (if clicked, show file location)
    int btnY = SCREEN_HEIGHT - 60;
    if (dataLogger.isReady()) {
        if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, BUTTON_HEIGHT)) {
            String filename = dataLogger.getCurrentRaceFile();
            if (filename.length() > 0) {
                showMessage("Gespeichert", filename, COLOR_SECONDARY);
            }
        }
    }
}

