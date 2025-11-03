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
            handleSettingsTouch(x, y);
            break;
        default:
            break;
    }
}

// ============================================================
// Screen-Specific Touch Handlers
// ============================================================

void handleHomeTouch(uint16_t x, uint16_t y) {
    int btnY = HEADER_HEIGHT + 12;
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
        if (itemNum >= 3) break;  // Max 3 visible
        
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
    
    int btnY = HEADER_HEIGHT + 15;
    
    // Team Name edit
    if (isTouchInRect(x, y, 10, btnY + 25, SCREEN_WIDTH - 20, 36)) {
        String newName = inputText("Team-Name", uiState.editingTeamName);
        if (newName.length() > 0) {
            uiState.editingTeamName = newName;
            uiState.needsRedraw = true;
        }
        return;
    }
    
    btnY += 70;
    
    // Beacon assignment
    if (isTouchInRect(x, y, 10, btnY + 25, SCREEN_WIDTH - 20, 36)) {
        uiState.previousScreen = SCREEN_TEAM_EDIT;
        uiState.currentScreen = SCREEN_TEAM_BEACON_ASSIGN;
        uiState.needsRedraw = true;
        
        // Start BLE scanning
        if (!bleScanner.isScanning()) {
            bleScanner.startScan();
        }
        return;
    }
    
    btnY += 70;
    
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
                    // WICHTIG: MAC-Adresse speichern, nicht UUID!
                    team->beaconUUID = nearest->macAddress;
                    
                    // Save to NVS
                    if (persistence.isInitialized()) {
                        persistence.saveTeams(lapCounter);
                    }
                    
                    Serial.printf("[Team %u] Beacon zugeordnet: MAC=%s (UUID=%s)\n", 
                                 uiState.editingTeamId, nearest->macAddress.c_str(), nearest->uuid.c_str());
                    
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
        return a.rssi > b.rssi;  // H?herer RSSI = st?rker = n?her
    });
    
    int y_start = HEADER_HEIGHT + 30;
    int itemHeight = 34;  // Kompakter: 42 -> 34
    
    int index = 0;
    for (auto& beacon : beacons) {
        if (index >= 8) break;  // Max 8 (gleich wie Drawing)
        
        int itemY = y_start + (index * itemHeight);
        
        if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 20, 32)) {  // Kompakter: 38 -> 32
            Serial.printf("[Touch] Beacon #%d angeklickt: MAC=%s UUID=%s (RSSI=%d) at y=%d\n", 
                         index, beacon.macAddress.c_str(), beacon.uuid.c_str(), beacon.rssi, itemY);
            
            float dist = BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower);
            
            if (dist < 1.0) {
                // Assign this beacon to the current team (using editingTeamId)
                TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
                
                if (team) {
                    // WICHTIG: MAC-Adresse speichern, nicht UUID!
                    team->beaconUUID = beacon.macAddress;
                    
                    // Save to NVS
                    if (persistence.isInitialized()) {
                        persistence.saveTeams(lapCounter);
                    }
                    
                    Serial.printf("[Team %u] Beacon zugeordnet: MAC=%s (UUID=%s)\n", 
                                 uiState.editingTeamId, beacon.macAddress.c_str(), beacon.uuid.c_str());
                    
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
    
    // Zeit-Anzeige klicken f?r manuellen Input
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
        uiState.resultsPage = 0;  // Reset to current race
        uiState.currentScreen = SCREEN_HOME;
        uiState.needsRedraw = true;
        return;
    }
    
    // Navigation buttons
    int btnY = SCREEN_HEIGHT - 50;
    int btnW = 60;
    
    // Previous button
    if (isTouchInRect(x, y, 10, btnY, btnW, 30)) {
        if (uiState.resultsPage > 0) {
            uiState.resultsPage--;
            uiState.needsRedraw = true;
        }
        return;
    }
    
    // Next button
    if (isTouchInRect(x, y, SCREEN_WIDTH - btnW - 10, btnY, btnW, 30)) {
        // Check max races
        String raceFiles = dataLogger.getRaceFileList(10);
        uint8_t maxRaces = 1;
        if (raceFiles.length() > 0) {
            int count = 1;
            int pos = 0;
            while ((pos = raceFiles.indexOf('\n', pos)) >= 0) {
                count++;
                pos++;
            }
            maxRaces = count;
            if (dataLogger.getCurrentRaceFile().length() == 0) {
                maxRaces = count;
            } else {
                maxRaces = count;
            }
        }
        
        if (uiState.resultsPage < maxRaces - 1) {
            uiState.resultsPage++;
            uiState.needsRedraw = true;
        }
        return;
    }
}

void handleSettingsTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
        uiState.currentScreen = SCREEN_HOME;
        uiState.needsRedraw = true;
        return;
    }
    
    int yStart = HEADER_HEIGHT + 8 + 12 + 10;  // Title + "NAH:" label
    int btnW = 40;
    int valW = 70;
    int centerX = SCREEN_WIDTH / 2;
    int totalW = btnW + 5 + valW + 5 + btnW;
    int startX = centerX - totalW / 2;
    
    // === RSSI NEAR Controls ===
    int yNear = yStart;
    
    // "-" Button (NEAR)
    if (isTouchInRect(x, y, startX, yNear, btnW, 30)) {
        if (lapRssiNear > MIN_RSSI) {
            lapRssiNear -= 5;
            
            // Save to NVS
            if (persistence.isInitialized()) {
                persistence.saveRssiThresholds(lapRssiNear, lapRssiFar);
            }
            
            Serial.printf("[Settings] RSSI NEAR: %d dBm\n", lapRssiNear);
            uiState.needsRedraw = true;
        }
        return;
    }
    
    // "+" Button (NEAR)
    if (isTouchInRect(x, y, startX + btnW + 5 + valW + 5, yNear, btnW, 30)) {
        if (lapRssiNear < MAX_RSSI) {
            lapRssiNear += 5;
            
            // Save to NVS
            if (persistence.isInitialized()) {
                persistence.saveRssiThresholds(lapRssiNear, lapRssiFar);
            }
            
            Serial.printf("[Settings] RSSI NEAR: %d dBm\n", lapRssiNear);
            uiState.needsRedraw = true;
        }
        return;
    }
    
    // === RSSI FAR Controls ===
    int yFar = yStart + 38 + 10;  // NEAR controls + "WEG:" label
    
    // "-" Button (FAR)
    if (isTouchInRect(x, y, startX, yFar, btnW, 30)) {
        if (lapRssiFar > MIN_RSSI) {
            lapRssiFar -= 5;
            
            // Save to NVS
            if (persistence.isInitialized()) {
                persistence.saveRssiThresholds(lapRssiNear, lapRssiFar);
            }
            
            Serial.printf("[Settings] RSSI FAR: %d dBm\n", lapRssiFar);
            uiState.needsRedraw = true;
        }
        return;
    }
    
    // "+" Button (FAR)
    if (isTouchInRect(x, y, startX + btnW + 5 + valW + 5, yFar, btnW, 30)) {
        if (lapRssiFar < MAX_RSSI) {
            lapRssiFar += 5;
            
            // Save to NVS
            if (persistence.isInitialized()) {
                persistence.saveRssiThresholds(lapRssiNear, lapRssiFar);
            }
            
            Serial.printf("[Settings] RSSI FAR: %d dBm\n", lapRssiFar);
            uiState.needsRedraw = true;
        }
        return;
    }
    
    // === SD Format Button ===
    if (dataLogger.isReady()) {
        int yFormat = HEADER_HEIGHT + 8 + 18 + 33 + 38 + 8 + 18 + 18 + 18;
        if (isTouchInRect(x, y, SCREEN_WIDTH - 110, yFormat - 2, 100, 25)) {
            // Confirmation dialog
            tft.fillScreen(BACKGROUND_COLOR);
            tft.setTextColor(TFT_RED);
            tft.setTextSize(2);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("WARNUNG!", SCREEN_WIDTH / 2, 60);
            
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(1);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("Alle Daten werden", SCREEN_WIDTH / 2, 100);
            tft.drawString("geloscht!", SCREEN_WIDTH / 2, 115);
            
            tft.setTextSize(1);
            tft.setCursor(10, 140);
            tft.print("Fortfahren?");
            
            // Buttons
            drawButton(10, 180, 90, 30, "Abbrechen", COLOR_DANGER);
            drawButton(SCREEN_WIDTH - 100, 180, 90, 30, "Formatieren", COLOR_WARNING);
            
            // Wait for confirmation
            bool confirmed = false;
            bool cancelled = false;
            uint32_t waitStart = millis();
            
            while (!confirmed && !cancelled && (millis() - waitStart < 30000)) {
                uint16_t tx, ty;
                if (getTouchCoordinates(&tx, &ty)) {
                    delay(50);
                    while (getTouchCoordinates(&tx, &ty)) delay(10);
                    
                    // Cancel button
                    if (isTouchInRect(tx, ty, 10, 180, 90, 30)) {
                        cancelled = true;
                        break;
                    }
                    
                    // Confirm button
                    if (isTouchInRect(tx, ty, SCREEN_WIDTH - 100, 180, 90, 30)) {
                        confirmed = true;
                        break;
                    }
                }
                delay(50);
            }
            
            if (confirmed) {
                Serial.println("[Settings] Formatting SD card...");
                if (dataLogger.formatSD()) {
                    showMessage("Erfolg", "SD-Karte formatiert", COLOR_SECONDARY);
                    // Reinitialize SD
                    dataLogger.begin(SD_CS_PIN);
                } else {
                    showMessage("Fehler", "Formatierung fehlgeschlagen", COLOR_DANGER);
                }
            }
            
            uiState.needsRedraw = true;
            return;
        }
    }
}

