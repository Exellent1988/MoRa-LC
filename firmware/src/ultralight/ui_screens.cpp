#include "ui_screens.h"
#include "DataLogger.h"
#include "ui_keyboard.h"
#include <algorithm>

extern DataLogger dataLogger;
extern bool raceRunning;
extern uint32_t raceStartTime;
extern uint32_t raceDuration;
extern String currentRaceName;

UIState uiState;

// ============================================================
// Screen Drawing
// ============================================================

void drawHomeScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader(DEVICE_NAME, false);
    
    int y = HEADER_HEIGHT + 12;
    int btnWidth = SCREEN_WIDTH - 2 * BUTTON_MARGIN;
    
    // Button: Neues Rennen
    drawButton(BUTTON_MARGIN, y, btnWidth, BUTTON_HEIGHT, "Neues Rennen", COLOR_SECONDARY);
    y += BUTTON_HEIGHT + BUTTON_MARGIN;
    
    // Button: Teams verwalten
    drawButton(BUTTON_MARGIN, y, btnWidth, BUTTON_HEIGHT, "Teams verwalten");
    y += BUTTON_HEIGHT + BUTTON_MARGIN;
    
    // Button: Ergebnisse
    drawButton(BUTTON_MARGIN, y, btnWidth, BUTTON_HEIGHT, "Ergebnisse");
    y += BUTTON_HEIGHT + BUTTON_MARGIN;
    
    // Button: Einstellungen
    drawButton(BUTTON_MARGIN, y, btnWidth, BUTTON_HEIGHT, "Einstellungen");
    
    // Footer: Status (kompakt)
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor(10, SCREEN_HEIGHT - 12);
    tft.printf("T:%u SD:%s BLE:%s", 
               lapCounter.getTeamCount(),
               dataLogger.isReady() ? "OK" : "-",
               bleScanner.isScanning() ? "ON" : "-");
}

void drawTeamsScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("Teams", true);
    
    auto teams = lapCounter.getAllTeams();
    int y = HEADER_HEIGHT + 10;
    
    if (teams.empty()) {
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextSize(2);
        tft.setCursor(20, y + 40);
        tft.print("Keine Teams");
        tft.setCursor(20, y + 70);
        tft.setTextSize(1);
        tft.print("Tippe unten zum Hinzufugen");
    } else {
        int displayCount = 0;
        for (auto* team : teams) {
            if (displayCount >= 3) break;  // Max 3 sichtbar (Platz für Button)
            
            tft.fillRoundRect(10, y, SCREEN_WIDTH - 20, LIST_ITEM_HEIGHT, 3, COLOR_BUTTON);
            tft.setTextColor(TFT_BLACK);
            tft.setTextSize(2);
            tft.setCursor(15, y + 5);
            tft.printf("%u. %s", team->teamId, team->teamName.c_str());
            
            tft.setTextSize(1);
            tft.setCursor(15, y + 25);
            if (team->beaconUUID.length() > 0) {
                tft.printf("Beacon: %.8s...", team->beaconUUID.c_str());
            } else {
                tft.print("Kein Beacon");
            }
            
            y += LIST_ITEM_HEIGHT + 5;
            displayCount++;
        }
    }
    
    // Add button
    int btnY = SCREEN_HEIGHT - 60;
    drawButton(BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, 
              BUTTON_HEIGHT, "+ Neues Team", COLOR_SECONDARY);
}

void drawTeamEditScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("Team bearbeiten", true);
    
    int y = HEADER_HEIGHT + 15;
    
    // Team Name
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, y);
    tft.print("Name:");
    
    tft.fillRoundRect(10, y + 25, SCREEN_WIDTH - 20, 36, 5, COLOR_BUTTON);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(15, y + 33);
    tft.print(uiState.editingTeamName.length() > 0 ? uiState.editingTeamName : "Team eingeben");
    
    y += 70;
    
    // Beacon Assignment
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(10, y);
    tft.print("Beacon:");
    
    TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
    String beaconText = "Nicht zugeordnet";
    if (team && team->beaconUUID.length() > 0) {
        beaconText = team->beaconUUID.substring(0, 12) + "...";
    }
    
    drawButton(10, y + 25, SCREEN_WIDTH - 20, 36, beaconText);
    
    y += 70;
    
    // Buttons
    drawButton(10, y, (SCREEN_WIDTH - 30) / 2, BUTTON_HEIGHT, "Speichern", COLOR_SECONDARY);
    drawButton((SCREEN_WIDTH + 10) / 2, y, (SCREEN_WIDTH - 30) / 2, BUTTON_HEIGHT, "Loschen", COLOR_DANGER);
}

void drawTeamBeaconAssignScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("Beacon zuordnen", true);
    
    int y = HEADER_HEIGHT + 10;
    
    // Anleitung
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, y);
    tft.print("Methode 1: Halte Mofa mit Beacon");
    tft.setCursor(10, y + 12);
    tft.print("direkt vor Display (< 1m)");
    
    y += 30;  // Kompakter: 45 -> 30
    
    // Nearest Beacon
    BeaconData* nearest = bleScanner.getNearestBeacon();
    if (nearest) {
        float dist = BLEScanner::rssiToDistance(nearest->rssi, nearest->txPower);
        
        tft.fillRoundRect(10, y, SCREEN_WIDTH - 20, 55, 5, 
                         dist < 1.0 ? COLOR_SECONDARY : COLOR_BUTTON);
        tft.setTextColor(TFT_BLACK);
        tft.setTextSize(1);  // Kleiner: 2 -> 1
        tft.setCursor(15, y + 5);
        tft.print("Nachster Beacon:");
        
        tft.setTextSize(1);
        tft.setCursor(15, y + 20);
        // IMMER MAC-Adresse anzeigen (für Konsistenz)
        tft.printf("MAC: %s", nearest->macAddress.c_str());
        tft.setCursor(15, y + 35);
        tft.printf("RSSI: %d dBm | %.2fm", nearest->rssi, dist);
        
        y += 60;  // Kompakter: 70 -> 60
        
        if (dist < 1.0) {
            // Zwei Buttons nebeneinander
            int btnW = (SCREEN_WIDTH - 30) / 2;
            drawButton(10, y, btnW, BUTTON_HEIGHT, "Zuordnen", COLOR_SECONDARY);
            drawButton(20 + btnW, y, btnW, BUTTON_HEIGHT, "Liste");
            y += BUTTON_HEIGHT + 10;
        } else {
            y += 10;
        }
    } else {
        // "Kein Beacon" Box - kompakter und höher
        tft.fillRoundRect(10, y, SCREEN_WIDTH - 20, 50, 5, TFT_DARKGREY);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(20, y + 10);
        tft.print("Kein Beacon");
        tft.setCursor(20, y + 30);
        tft.setTextSize(1);
        tft.print("gefunden...");
        y += 60;
    }
    
    // Separator
    y = SCREEN_HEIGHT - 70;
    tft.drawLine(10, y, SCREEN_WIDTH - 10, y, TFT_DARKGREY);
    
    // Alternative: Manual list (immer sichtbar)
    y += 10;
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, y);
    tft.print("Alle Beacons:");
    
    y += 15;
    drawButton(10, y, SCREEN_WIDTH - 20, BUTTON_HEIGHT, "Beacon-Liste anzeigen");
}

void drawBeaconListScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("Beacon-Liste", true);
    
    auto beacons = bleScanner.getBeacons();
    
    // Debug: Log beacon count
    Serial.printf("[UI] drawBeaconListScreen: %u beacons from scanner\n", beacons.size());
    for (auto& b : beacons) {
        Serial.printf("[UI]   - MAC=%s UUID=%s: RSSI=%d\n", b.macAddress.c_str(), b.uuid.c_str(), b.rssi);
    }
    
    // Sort by RSSI (strongest first)
    std::sort(beacons.begin(), beacons.end(), [](const BeaconData& a, const BeaconData& b) {
        return a.rssi > b.rssi;  // Höherer RSSI = stärker = näher
    });
    
    int y = HEADER_HEIGHT + 10;
    
    if (beacons.empty()) {
        Serial.println("[UI] No beacons to display!");
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextSize(2);
        tft.setCursor(20, y + 40);
        tft.print("Keine Beacons");
        tft.setCursor(20, y + 70);
        tft.setTextSize(1);
        tft.print("gefunden...");
    } else {
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.setCursor(10, y);
        tft.printf("Gefunden: %u Beacon(s)", beacons.size());
        Serial.printf("[UI] Displaying up to 8 of %u beacons (starting at y=%d)\n", beacons.size(), y);
        
        y += 20;
        
        int displayCount = 0;
        for (auto& beacon : beacons) {
            if (displayCount >= 8) {
                Serial.println("[UI] Reached display limit (8)");
                break;  // Max 8 sichtbar
            }
            if (y > SCREEN_HEIGHT - 10) {
                Serial.printf("[UI] Reached screen height limit at y=%d\n", y);
                break;
            }
            
            Serial.printf("[UI] Drawing beacon #%d: MAC=%s UUID=%s (RSSI=%d) at y=%d\n", 
                         displayCount, beacon.macAddress.c_str(), beacon.uuid.c_str(), beacon.rssi, y);
            
            float dist = BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower);
            
            uint16_t bgColor = dist < 1.0 ? COLOR_SECONDARY : COLOR_BUTTON;
            tft.fillRoundRect(10, y, SCREEN_WIDTH - 20, 32, 3, bgColor);  // Kompakter: 38 -> 32
            tft.setTextColor(TFT_BLACK);
            tft.setTextSize(1);
            tft.setCursor(15, y + 3);
            
            // IMMER MAC-Adresse anzeigen (für Zuordnung!)
            tft.setTextColor(TFT_BLACK);
            tft.printf("%s", beacon.macAddress.c_str());
            
            // Optional: iBeacon Indicator
            if (beacon.uuid.length() > 30) {
                tft.setTextColor(TFT_DARKGREY);
                tft.printf(" (iBeacon)");
            }
            
            tft.setTextColor(TFT_BLACK);
            tft.setCursor(15, y + 13);
            tft.printf("%d dBm | %.2fm", beacon.rssi, dist);
            
            tft.setCursor(15, y + 23);
            if (dist < 1.0) {
                tft.setTextColor(TFT_DARKGREEN);
                tft.print("< Tippen zum Zuordnen!");
            } else {
                tft.setTextColor(TFT_DARKGREY);
                tft.print("zu weit");
            }
            
            y += 34;  // Kompakter: 42 -> 34
            displayCount++;
        }
    }
}

void drawRaceSetupScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("Rennen einrichten", true);
    
    int y = HEADER_HEIGHT + 10;
    
    // === Race Name ===
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setTextSize(1);
    tft.setCursor(10, y);
    tft.print("RENNEN-NAME");
    
    tft.fillRoundRect(10, y + 12, SCREEN_WIDTH - 20, 32, 5, COLOR_BUTTON);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(15, y + 20);
    if (uiState.raceName.length() > 0) {
        tft.print(uiState.raceName);
    } else {
        tft.setTextColor(TFT_DARKGREY);
        tft.print("Tippen...");
    }
    
    y += 50;
    
    // === Duration mit +/- Controls ===
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setTextSize(1);
    tft.setCursor(10, y);
    tft.print("DAUER (MINUTEN)");
    
    y += 12;
    
    // Zeit-Anzeige (groß in der Mitte)
    int timeBoxX = 60;
    int timeBoxW = 120;
    tft.fillRoundRect(timeBoxX, y, timeBoxW, 38, 5, COLOR_SECONDARY);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(3);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(String(uiState.raceDuration), timeBoxX + timeBoxW/2, y + 19);
    
    // "-5" Button (links)
    int btnSize = 38;
    tft.fillRoundRect(10, y, 45, btnSize, 5, COLOR_BUTTON);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("-5", 32, y + btnSize/2);
    
    // "+5" Button (rechts)
    tft.fillRoundRect(185, y, 45, btnSize, 5, COLOR_BUTTON);
    tft.drawString("+5", 207, y + btnSize/2);
    
    // "+15" Button (ganz rechts)
    tft.fillRoundRect(235, y, 45, btnSize, 5, COLOR_BUTTON);
    tft.setTextSize(1);
    tft.drawString("+15", 257, y + btnSize/2);
    
    y += 48;
    
    // === Teams ===
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(10, y);
    
    int teamCount = lapCounter.getTeamCount();
    if (teamCount == 0) {
        tft.print("KEINE TEAMS");
    } else {
        tft.printf("TEAMS (%d)", teamCount);
    }
    
    y += 12;
    
    // Team Liste (kompakt)
    if (teamCount > 0) {
        for (uint8_t i = 1; i <= MAX_TEAMS && i <= teamCount; i++) {
            TeamData* team = lapCounter.getTeam(i);
            if (!team) continue;
            
            if (y > SCREEN_HEIGHT - 60) break;  // Platz für Start-Button
            
            // Team Box
            uint16_t bgColor = uiState.selectedTeams[i-1] ? COLOR_SECONDARY : TFT_DARKGREY;
            tft.fillRoundRect(10, y, SCREEN_WIDTH - 20, 20, 3, bgColor);
            
            // Checkbox
            tft.fillCircle(20, y + 10, 4, uiState.selectedTeams[i-1] ? TFT_BLACK : TFT_WHITE);
            
            // Team Name
            tft.setTextColor(TFT_BLACK);
            tft.setTextSize(1);
            tft.setCursor(30, y + 6);
            tft.print(team->teamName);
            
            y += 23;
        }
    } else {
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextSize(1);
        tft.setCursor(20, y + 5);
        tft.print("Zuerst Teams anlegen!");
    }
    
    // === Start Button ===
    int btnY = SCREEN_HEIGHT - 50;
    uint16_t btnColor = (teamCount > 0) ? COLOR_SECONDARY : TFT_DARKGREY;
    drawButton(10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT, "RENNEN STARTEN", btnColor);
}

void drawRaceRunningScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    
    // Header with Time
    uint32_t elapsed = millis() - raceStartTime;
    uint32_t remaining = raceDuration - elapsed;
    uint32_t minutes = remaining / 60000;
    uint32_t seconds = (remaining % 60000) / 1000;
    
    tft.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_PRIMARY);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 8);
    tft.printf("Zeit: %02lu:%02lu", minutes, seconds);
    
    // Leaderboard
    auto leaderboard = lapCounter.getLeaderboard(true);
    
    int y = HEADER_HEIGHT + 10;
    int pos = 1;
    
    for (auto* team : leaderboard) {
        if (y > SCREEN_HEIGHT - 80) break;
        
        // Position & Name
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setCursor(10, y);
        tft.printf("%u. %s", pos, team->teamName.c_str());
        
        // Laps
        tft.setCursor(220, y);
        uint16_t laps = team->lapCount > 0 ? team->lapCount - 1 : 0;
        tft.printf("%uR", laps);
        
        // Last Lap Time
        if (!team->laps.empty()) {
            LapTime& lastLap = team->laps.back();
            tft.setTextSize(1);
            tft.setCursor(20, y + 18);
            tft.printf("Letzte: %lu.%03lu s", 
                      lastLap.duration / 1000, lastLap.duration % 1000);
        }
        
        y += 35;
        pos++;
    }
    
    // Buttons
    int btnY = SCREEN_HEIGHT - 50;
    drawButton(10, btnY, 100, 40, "Pause", COLOR_WARNING);
    drawButton(120, btnY, 100, 40, "Stop", COLOR_DANGER);
}

void drawRacePausedScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("RENNEN PAUSIERT", false);
    
    int y = HEADER_HEIGHT + 40;
    
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.setCursor(50, y);
    tft.print("PAUSE");
    
    y += 60;
    
    // Time
    uint32_t elapsed = millis() - raceStartTime;
    uint32_t minutes = elapsed / 60000;
    uint32_t seconds = (elapsed % 60000) / 1000;
    
    tft.setTextSize(2);
    tft.setCursor(60, y);
    tft.printf("Zeit: %02lu:%02lu", minutes, seconds);
    
    // Buttons
    y = SCREEN_HEIGHT - 110;
    drawButton(BUTTON_MARGIN, y, SCREEN_WIDTH - 2*BUTTON_MARGIN, 
              BUTTON_HEIGHT, "Weitermachen", COLOR_SECONDARY);
    
    y += BUTTON_HEIGHT + 10;
    drawButton(BUTTON_MARGIN, y, SCREEN_WIDTH - 2*BUTTON_MARGIN, 
              BUTTON_HEIGHT, "Rennen beenden", COLOR_DANGER);
}

void drawRaceResultsScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("Ergebnisse", true);
    
    // Get list of race files
    String raceFiles = dataLogger.getRaceFileList(10);
    
    // Parse race files
    struct RaceFile {
        String filename;
        int index;
    };
    RaceFile races[4];  // 0 = current, 1-3 = last 3
    uint8_t raceCount = 0;
    
    // Current race is always index 0
    if (dataLogger.getCurrentRaceFile().length() > 0) {
        races[0].filename = dataLogger.getCurrentRaceFile();
        races[0].index = 0;
        raceCount = 1;
    }
    
    // Parse historical races from file list
    int startPos = 0;
    int newlinePos = raceFiles.indexOf('\n', startPos);
    uint8_t fileIndex = 1;
    
    while (newlinePos >= 0 && fileIndex < 4 && raceCount < 4) {
        String filename = raceFiles.substring(startPos, newlinePos);
        if (filename.length() > 0 && (raceCount == 0 || filename != races[0].filename)) {
            races[raceCount].filename = "/races/" + filename;
            races[raceCount].index = fileIndex;
            raceCount++;
            fileIndex++;
        }
        startPos = newlinePos + 1;
        newlinePos = raceFiles.indexOf('\n', startPos);
    }
    
    // If no newline at end, last file
    if (startPos < raceFiles.length() && raceCount < 4) {
        String filename = raceFiles.substring(startPos);
        if (filename.length() > 0 && (raceCount == 0 || filename != races[0].filename)) {
            races[raceCount].filename = "/races/" + filename;
            races[raceCount].index = fileIndex;
            raceCount++;
        }
    }
    
    // Show current race selection
    int y = HEADER_HEIGHT + 10;
    
    if (raceCount == 0) {
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextSize(2);
        tft.setCursor(40, y + 60);
        tft.print("Keine Rennen");
    } else {
        // Show which race we're viewing
        tft.setTextColor(TFT_LIGHTGREY);
        tft.setTextSize(1);
        tft.setCursor(10, y);
        
        if (uiState.resultsPage == 0 && raceCount > 0) {
            tft.print("Aktuelles Rennen:");
        } else if (uiState.resultsPage < raceCount) {
            tft.printf("Rennen %u/%u:", uiState.resultsPage, raceCount - 1);
        }
        
        y += 15;
        
        // Load and display race results
        if (uiState.resultsPage < raceCount) {
            String raceFile = races[uiState.resultsPage].filename;
            displayRaceResultsFromFile(raceFile, y);
        }
        
        // Navigation buttons if multiple races
        if (raceCount > 1) {
            int btnY = SCREEN_HEIGHT - 50;
            int btnW = 60;
            
            // Previous button
            if (uiState.resultsPage > 0) {
                drawButton(10, btnY, btnW, 30, "< Zuruck", COLOR_BUTTON);
            }
            
            // Next button
            if (uiState.resultsPage < raceCount - 1) {
                drawButton(SCREEN_WIDTH - btnW - 10, btnY, btnW, 30, "Vor >", COLOR_BUTTON);
            }
            
            // Page indicator
            tft.setTextColor(TFT_DARKGREY);
            tft.setTextSize(1);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(String(uiState.resultsPage + 1) + "/" + String(raceCount), 
                          SCREEN_WIDTH / 2, btnY + 15);
        }
    }
}

void displayRaceResultsFromFile(const String& filename, int startY) {
    if (!dataLogger.exists(filename)) {
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextSize(1);
        tft.setCursor(10, startY + 20);
        tft.print("Datei nicht gefunden");
        return;
    }
    
    // Read CSV file
    String content = dataLogger.readFile(filename);
    if (content.length() == 0) {
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextSize(1);
        tft.setCursor(10, startY + 20);
        tft.print("Keine Daten");
        return;
    }
    
    // Parse CSV and build leaderboard
    // Format: Team ID,Team Name,Lap Number,Timestamp,Duration,Time of Day
    struct TeamStats {
        uint8_t teamId;
        String teamName;
        uint16_t lapCount;
        uint32_t bestLapDuration;
        uint32_t totalDuration;
    };
    
    std::map<uint8_t, TeamStats> teamStats;
    
    int lineStart = 0;
    bool firstLine = true;
    
    while (lineStart < content.length()) {
        int lineEnd = content.indexOf('\n', lineStart);
        if (lineEnd < 0) lineEnd = content.length();
        
        String line = content.substring(lineStart, lineEnd);
        
        if (firstLine) {
            firstLine = false;
            lineStart = lineEnd + 1;
            continue;  // Skip header
        }
        
        // Parse CSV line
        int pos = 0;
        int commaPos = line.indexOf(',', pos);
        if (commaPos < 0) {
            lineStart = lineEnd + 1;
            continue;
        }
        
        uint8_t teamId = line.substring(pos, commaPos).toInt();
        
        pos = commaPos + 1;
        commaPos = line.indexOf(',', pos);
        if (commaPos < 0) {
            lineStart = lineEnd + 1;
            continue;
        }
        String teamName = line.substring(pos, commaPos);
        
        pos = commaPos + 1;
        commaPos = line.indexOf(',', pos);
        if (commaPos < 0) {
            lineStart = lineEnd + 1;
            continue;
        }
        uint16_t lapNumber = line.substring(pos, commaPos).toInt();
        
        pos = commaPos + 1;
        commaPos = line.indexOf(',', pos);
        if (commaPos < 0) {
            lineStart = lineEnd + 1;
            continue;
        }
        
        pos = commaPos + 1;
        commaPos = line.indexOf(',', pos);
        if (commaPos < 0) {
            lineStart = lineEnd + 1;
            continue;
        }
        uint32_t duration = line.substring(pos, commaPos).toULong();
        
        // Update stats
        if (teamStats.find(teamId) == teamStats.end()) {
            TeamStats stats;
            stats.teamId = teamId;
            stats.teamName = teamName;
            stats.lapCount = 0;
            stats.bestLapDuration = UINT32_MAX;
            stats.totalDuration = 0;
            teamStats[teamId] = stats;
        }
        
        TeamStats& stats = teamStats[teamId];
        stats.lapCount++;
        stats.totalDuration += duration;
        if (duration < stats.bestLapDuration) {
            stats.bestLapDuration = duration;
        }
        
        lineStart = lineEnd + 1;
    }
    
    // Convert to vector and sort
    std::vector<TeamStats*> leaderboard;
    for (auto& pair : teamStats) {
        leaderboard.push_back(&pair.second);
    }
    
    std::sort(leaderboard.begin(), leaderboard.end(), 
              [](const TeamStats* a, const TeamStats* b) {
                  if (a->lapCount != b->lapCount) {
                      return a->lapCount > b->lapCount;
                  }
                  return a->bestLapDuration < b->bestLapDuration;
              });
    
    // Display leaderboard
    int y = startY + 5;
    int pos = 1;
    
    for (auto* stats : leaderboard) {
        if (y > SCREEN_HEIGHT - 80) break;
        
        const char* medal = "";
        uint16_t medalColor = TFT_WHITE;
        if (pos == 1) { medal = "1."; medalColor = TFT_GOLD; }
        else if (pos == 2) { medal = "2."; medalColor = TFT_SILVER; }
        else if (pos == 3) { medal = "3."; medalColor = 0xCD7F32; }
        
        tft.setTextColor(medalColor);
        tft.setTextSize(2);
        tft.setCursor(10, y);
        tft.print(medal);
        
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(40, y);
        tft.print(stats->teamName);
        
        tft.setTextSize(1);
        tft.setCursor(20, y + 18);
        tft.printf("%u Runden", stats->lapCount);
        
        if (stats->bestLapDuration < UINT32_MAX) {
            tft.setCursor(120, y + 18);
            tft.printf("Beste: %lu.%03lu s", 
                      stats->bestLapDuration / 1000, 
                      stats->bestLapDuration % 1000);
        }
        
        y += 38;
        pos++;
    }
}

void drawSettingsScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("Einstellungen", true);
    
    int y = HEADER_HEIGHT + 8;
    
    // === RSSI Thresholds (wichtigste Einstellung!) ===
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, y);
    tft.print("LAP DETECTION (On-the-fly!):");
    
    y += 15;
    
    // RSSI NEAR (NAH)
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(10, y);
    tft.print("NAH (Ziellinie):");
    
    int btnW = 35;
    int valW = 60;
    int xVal = 130;
    
    // "-" Button
    drawButton(xVal - btnW - 5, y - 3, btnW, 28, "-", COLOR_BUTTON);
    
    // Value Display
    tft.fillRoundRect(xVal, y - 3, valW, 28, 3, COLOR_SECONDARY);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(String(lapRssiNear), xVal + valW/2, y + 11);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    
    // "+" Button
    drawButton(xVal + valW + 5, y - 3, btnW, 28, "+", COLOR_BUTTON);
    
    y += 33;
    
    // RSSI FAR (WEG)
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(10, y);
    tft.print("WEG (weggefahren):");
    
    // "-" Button
    drawButton(xVal - btnW - 5, y - 3, btnW, 28, "-", COLOR_BUTTON);
    
    // Value Display
    tft.fillRoundRect(xVal, y - 3, valW, 28, 3, COLOR_SECONDARY);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(String(lapRssiFar), xVal + valW/2, y + 11);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    
    // "+" Button
    drawButton(xVal + valW + 5, y - 3, btnW, 28, "+", COLOR_BUTTON);
    
    y += 38;
    
    // Separator
    tft.drawLine(10, y, SCREEN_WIDTH - 10, y, TFT_DARKGREY);
    y += 8;
    
    // === BLE Scanner ===
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(10, y);
    tft.print("BLE Scanner:");
    
    tft.fillCircle(120, y + 4, 4, bleScanner.isScanning() ? COLOR_SECONDARY : TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(130, y);
    tft.print(bleScanner.isScanning() ? "Aktiv" : "Inaktiv");
    
    y += 18;
    
    // === SD Card ===
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(10, y);
    tft.print("SD-Karte:");
    
    tft.fillCircle(120, y + 4, 4, dataLogger.isReady() ? COLOR_SECONDARY : COLOR_DANGER);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(130, y);
    tft.print(dataLogger.isReady() ? "Bereit" : "Nicht verf.");
    
    y += 18;
    
    y += 18;
    
    // === SD Card Format Button ===
    if (dataLogger.isReady()) {
        tft.setTextColor(TFT_LIGHTGREY);
        tft.setCursor(10, y);
        tft.print("SD-Format:");
        
        drawButton(SCREEN_WIDTH - 110, y - 2, 100, 25, "Formatieren", COLOR_WARNING);
        y += 20;
    }
    
    // === Info ===
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor(10, y);
    tft.printf("Version: %s | RAM: %u KB", VERSION, ESP.getFreeHeap() / 1024);
}

// ============================================================
// Helper Functions
// ============================================================

void drawHeader(const String& title, bool showBack) {
    tft.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_PRIMARY);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 8);
    tft.print(title);
    
    if (showBack) {
        tft.setCursor(SCREEN_WIDTH - 50, 8);
        tft.print("[<]");
    }
}

void drawButton(int x, int y, int w, int h, const String& text, uint16_t color) {
    tft.fillRoundRect(x, y, w, h, 5, color);
    tft.setTextColor(TFT_BLACK);
    
    // Center text
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(text, x + w / 2, y + h / 2);
}

bool isTouchInRect(uint16_t tx, uint16_t ty, int x, int y, int w, int h) {
    return tx >= x && tx <= (x + w) && ty >= y && ty <= (y + h);
}

void showMessage(const String& title, const String& message, uint16_t color) {
    tft.fillScreen(BACKGROUND_COLOR);
    tft.setTextColor(color);
    tft.setTextSize(2);
    
    tft.setTextDatum(TC_DATUM);  // Top Center
    tft.drawString(title, SCREEN_WIDTH / 2, 80);
    
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(message, SCREEN_WIDTH / 2, 120);
    
    delay(2000);
    uiState.needsRedraw = true;
}

String inputText(const String& prompt, const String& defaultValue) {
    OnScreenKeyboard keyboard;
    String result = keyboard.getText(prompt, defaultValue, 20);
    uiState.needsRedraw = true;
    return result;
}

uint32_t inputNumber(const String& prompt, uint32_t defaultValue, uint32_t min, uint32_t max) {
    OnScreenKeyboard keyboard;
    uint32_t result = keyboard.getNumber(prompt, defaultValue, min, max);
    uiState.needsRedraw = true;
    return result;
}

