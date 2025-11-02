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
    
    int y = HEADER_HEIGHT + 20;
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
    
    // Footer: Status
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor(10, SCREEN_HEIGHT - 15);
    tft.printf("Teams: %u | SD: %s | BLE: %s", 
               lapCounter.getTeamCount(),
               dataLogger.isReady() ? "OK" : "NO",
               bleScanner.isScanning() ? "ON" : "OFF");
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
            if (displayCount >= 4) break;  // Max 4 sichtbar
            
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
    
    int y = HEADER_HEIGHT + 20;
    
    // Team Name
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, y);
    tft.print("Name:");
    
    tft.fillRoundRect(10, y + 25, SCREEN_WIDTH - 20, 40, 5, COLOR_BUTTON);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(15, y + 35);
    tft.print(uiState.editingTeamName.length() > 0 ? uiState.editingTeamName : "Team eingeben");
    
    y += 80;
    
    // Beacon Assignment
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(10, y);
    tft.print("Beacon:");
    
    TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
    String beaconText = "Nicht zugeordnet";
    if (team && team->beaconUUID.length() > 0) {
        beaconText = team->beaconUUID.substring(0, 12) + "...";
    }
    
    drawButton(10, y + 25, SCREEN_WIDTH - 20, 40, beaconText);
    
    y += 80;
    
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
        tft.printf("UUID: %.12s...", nearest->uuid.c_str());
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
        Serial.printf("[UI]   - %s: RSSI=%d\n", b.uuid.c_str(), b.rssi);
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
            
            Serial.printf("[UI] Drawing beacon #%d: %s (RSSI=%d) at y=%d\n", displayCount, beacon.uuid.c_str(), beacon.rssi, y);
            
            float dist = BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower);
            
            uint16_t bgColor = dist < 1.0 ? COLOR_SECONDARY : COLOR_BUTTON;
            tft.fillRoundRect(10, y, SCREEN_WIDTH - 20, 32, 3, bgColor);  // Kompakter: 38 -> 32
            tft.setTextColor(TFT_BLACK);
            tft.setTextSize(1);
            tft.setCursor(15, y + 3);
            
            // Show MAC address (or UUID) - kompakter
            if (beacon.uuid.length() > 30) {
                // iBeacon UUID (kurz anzeigen)
                tft.printf("iB: %.8s..", beacon.uuid.c_str());
            } else {
                // MAC address
                tft.printf("%s", beacon.uuid.c_str());
            }
            
            tft.setCursor(15, y + 14);
            tft.printf("%d dBm | %.2fm", beacon.rssi, dist);
            
            tft.setCursor(15, y + 24);
            if (dist < 1.0) {
                tft.setTextColor(TFT_DARKGREEN);
                tft.print("< Tippen!");
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
    
    int y = HEADER_HEIGHT + 10;
    
    auto leaderboard = lapCounter.getLeaderboard(true);
    
    if (leaderboard.empty()) {
        tft.setTextColor(TFT_DARKGREY);
        tft.setTextSize(2);
        tft.setCursor(40, y + 60);
        tft.print("Keine Daten");
    } else {
        int pos = 1;
        for (auto* team : leaderboard) {
            if (y > SCREEN_HEIGHT - 100) break;
            
            // Medal emoji simulation
            const char* medal = "";
            uint16_t medalColor = TFT_WHITE;
            if (pos == 1) { medal = "1."; medalColor = TFT_GOLD; }
            else if (pos == 2) { medal = "2."; medalColor = TFT_SILVER; }
            else if (pos == 3) { medal = "3."; medalColor = 0xCD7F32; }  // Bronze
            
            tft.setTextColor(medalColor);
            tft.setTextSize(2);
            tft.setCursor(10, y);
            tft.print(medal);
            
            tft.setTextColor(TFT_WHITE);
            tft.setCursor(40, y);
            tft.print(team->teamName);
            
            tft.setTextSize(1);
            tft.setCursor(20, y + 18);
            uint16_t laps = team->lapCount > 0 ? team->lapCount - 1 : 0;
            tft.printf("%u Runden", laps);
            
            if (team->bestLapDuration < UINT32_MAX) {
                tft.setCursor(120, y + 18);
                tft.printf("Beste: %lu.%03lu s", 
                          team->bestLapDuration / 1000, 
                          team->bestLapDuration % 1000);
            }
            
            y += 38;
            pos++;
        }
    }
    
    // Export Button
    int btnY = SCREEN_HEIGHT - 60;
    if (dataLogger.isReady()) {
        drawButton(BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, 
                  BUTTON_HEIGHT, "Auf SD gespeichert", COLOR_SECONDARY);
    }
}

void drawSettingsScreen() {
    tft.fillScreen(BACKGROUND_COLOR);
    drawHeader("Einstellungen", true);
    
    int y = HEADER_HEIGHT + 20;
    
    // BLE Settings
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, y);
    tft.print("BLE Scanner:");
    
    y += 30;
    drawButton(10, y, SCREEN_WIDTH - 20, 40, 
              bleScanner.isScanning() ? "Aktiv" : "Inaktiv",
              bleScanner.isScanning() ? COLOR_SECONDARY : COLOR_BUTTON);
    
    y += 60;
    
    // SD Card
    tft.setCursor(10, y);
    tft.print("SD-Karte:");
    
    y += 30;
    String sdStatus = dataLogger.isReady() ? "Bereit" : "Nicht verfugbar";
    drawButton(10, y, SCREEN_WIDTH - 20, 40, sdStatus,
              dataLogger.isReady() ? COLOR_SECONDARY : COLOR_DANGER);
    
    y += 60;
    
    // Info
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor(10, y);
    tft.printf("Version: %s", VERSION);
    tft.setCursor(10, y + 15);
    tft.printf("Freier RAM: %u KB", ESP.getFreeHeap() / 1024);
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

