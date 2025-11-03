#include "ui_screens.h"
#include "ui_helper.h"
#include "../../lib/DataLogger/DataLogger.h"
#include "../ultralight/persistence.h"
#include <algorithm>

// Forward declarations
extern DataLogger dataLogger;
extern PersistenceManager persistence;
extern bool raceRunning;
extern uint32_t raceStartTime;
extern uint32_t raceDuration;
extern String currentRaceName;
extern std::map<uint8_t, bool> beaconPresence;

// ============================================================
// Screen Drawing
// ============================================================

void drawHomeScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader(DEVICE_NAME, false);
    
    // Reduced button height to fit all 4 buttons on screen
    int btnHeight = 42;  // Slightly smaller to fit all buttons
    int btnSpacing = 8;  // Reduced spacing
    int y = HEADER_HEIGHT + 10;
    int btnWidth = SCREEN_WIDTH - 2 * BUTTON_MARGIN;
    
    // Button: Neues Rennen
    drawButton(BUTTON_MARGIN, y, btnWidth, btnHeight, "Neues Rennen", COLOR_SECONDARY);
    y += btnHeight + btnSpacing;
    
    // Button: Teams verwalten
    drawButton(BUTTON_MARGIN, y, btnWidth, btnHeight, "Teams verwalten", COLOR_BUTTON);
    y += btnHeight + btnSpacing;
    
    // Button: Ergebnisse
    drawButton(BUTTON_MARGIN, y, btnWidth, btnHeight, "Ergebnisse", COLOR_BUTTON);
    y += btnHeight + btnSpacing;
    
    // Button: Einstellungen
    drawButton(BUTTON_MARGIN, y, btnWidth, btnHeight, "Einstellungen", COLOR_BUTTON);
    
    // Footer: Status - kleiner, oben zwischen Header und Buttons
    lcd.setTextSize(1);  // Smaller text for footer
    lcd.setTextColor(TEXT_COLOR);
    lcd.setCursor(10, SCREEN_HEIGHT - 12);
    lcd.printf("T:%u SD:%s BLE:%s", 
               lapCounter.getTeamCount(),
               dataLogger.isReady() ? "OK" : "-",
               bleScanner.isScanning() ? "ON" : "-");
}

void drawTeamsScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Teams", true);
    
    auto teams = lapCounter.getAllTeams();
    int y = HEADER_HEIGHT + 10;
    
    if (teams.empty()) {
        // Größerer Text für bessere Lesbarkeit
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_LARGE);
        lcd.setTextDatum(TC_DATUM);
        lcd.drawString("Keine Teams", SCREEN_WIDTH / 2, y + 50);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.drawString("Tippe unten zum Hinzufugen", SCREEN_WIDTH / 2, y + 90);
    } else {
        int displayCount = 0;
        for (auto* team : teams) {
            // Check if there's space for this team + button (60px for button)
            if (y + LIST_ITEM_HEIGHT + BUTTON_MARGIN > SCREEN_HEIGHT - 60) break;
            
            // Team Item - kompakt aber lesbar
            lcd.fillRoundRect(10, y, SCREEN_WIDTH - 20, LIST_ITEM_HEIGHT, 5, COLOR_BUTTON);
            lcd.drawRoundRect(10, y, SCREEN_WIDTH - 20, LIST_ITEM_HEIGHT, 5, 0x4208);  // Border
            
            // Team Name - kompakt
            lcd.setTextColor(COLOR_BUTTON_TEXT);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(15, y + 6);
            lcd.printf("%u. %s", team->teamId, team->teamName.c_str());
            
            // Beacon Info - kompakter
            lcd.setTextSize(1);  // Smaller for beacon info
            lcd.setCursor(15, y + 26);
            if (team->beaconUUID.length() > 0) {
                lcd.printf("Beacon: %.8s...", team->beaconUUID.c_str());
            } else {
                lcd.print("Kein Beacon");
            }
            
            y += LIST_ITEM_HEIGHT + BUTTON_MARGIN;
            displayCount++;
        }
    }
    
    // Add button
    int btnY = SCREEN_HEIGHT - 60;
    drawButton(BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, 
              BUTTON_HEIGHT, "+ Neues Team", COLOR_SECONDARY);
}

void drawTeamEditScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Team bearbeiten", true);
    
    int y = HEADER_HEIGHT + 15;
    
    // Team Name
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(2);
    lcd.setCursor(10, y);
    lcd.print("Name:");
    
    lcd.fillRoundRect(10, y + 25, SCREEN_WIDTH - 20, 36, 5, COLOR_BUTTON);
    lcd.setTextColor(0x0000);  // Black
    lcd.setCursor(15, y + 33);
    lcd.print(uiState.editingTeamName.length() > 0 ? uiState.editingTeamName : "Team eingeben");
    
    y += 70;
    
    // Beacon Assignment
    lcd.setTextColor(TEXT_COLOR);
    lcd.setCursor(10, y);
    lcd.print("Beacon:");
    
    TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
    String beaconText = "Nicht zugeordnet";
    if (team && team->beaconUUID.length() > 0) {
        beaconText = team->beaconUUID.substring(0, 12) + "...";
    }
    
    drawButton(10, y + 25, SCREEN_WIDTH - 20, 36, beaconText, COLOR_BUTTON);
    
    y += 70;
    
    // Buttons
    drawButton(10, y, (SCREEN_WIDTH - 30) / 2, BUTTON_HEIGHT, "Speichern", COLOR_SECONDARY);
    drawButton((SCREEN_WIDTH + 10) / 2, y, (SCREEN_WIDTH - 30) / 2, BUTTON_HEIGHT, "Loschen", COLOR_DANGER);
}

void drawTeamBeaconAssignScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Beacon zuordnen", true);
    
    int y = HEADER_HEIGHT + 12;
    
    // Instruction - größer und dunkel
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, y);
    lcd.print("Halte Mofa mit Beacon");
    lcd.setCursor(10, y + 20);
    lcd.print("direkt vor Display (< 1m)");
    
    y += 35;
    
    // Nearest Beacon - größer
    BeaconData* nearest = bleScanner.getNearestBeacon();
    if (nearest) {
        float dist = BLEScanner::rssiToDistance(nearest->rssi, nearest->txPower);
        
        uint16_t bgColor = dist < 1.0 ? COLOR_SECONDARY : COLOR_BUTTON;
        int boxH = 65;
        lcd.fillRoundRect(10, y, SCREEN_WIDTH - 20, boxH, 8, bgColor);
        lcd.drawRoundRect(10, y, SCREEN_WIDTH - 20, boxH, 8, 0x4208);
        
        lcd.setTextColor(COLOR_BUTTON_TEXT);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.setCursor(15, y + 8);
        lcd.print("Nachster Beacon:");
        
        lcd.setCursor(15, y + 28);
        lcd.printf("MAC: %s", nearest->macAddress.c_str());
        lcd.setCursor(15, y + 48);
        lcd.printf("RSSI: %d dBm | %.2fm", nearest->rssi, dist);
        
        y += boxH + 12;
        
        if (dist < 1.0) {
            int btnW = (SCREEN_WIDTH - 30) / 2;
            drawButton(10, y, btnW, BUTTON_HEIGHT, "Zuordnen", COLOR_SECONDARY);
            drawButton(20 + btnW, y, btnW, BUTTON_HEIGHT, "Liste", COLOR_BUTTON);
            y += BUTTON_HEIGHT + 10;
        } else {
            y += 10;
        }
    } else {
        int boxH = 60;
        lcd.fillRoundRect(10, y, SCREEN_WIDTH - 20, boxH, 8, COLOR_BUTTON);
        lcd.drawRoundRect(10, y, SCREEN_WIDTH - 20, boxH, 8, 0x4208);
        lcd.setTextColor(COLOR_BUTTON_TEXT);
        lcd.setTextSize(TEXT_SIZE_LARGE);
        lcd.setTextDatum(MC_DATUM);
        lcd.drawString("Kein Beacon", SCREEN_WIDTH / 2, y + 15);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.drawString("gefunden...", SCREEN_WIDTH / 2, y + 40);
        y += boxH + 12;
    }
    
    // Separator
    y = SCREEN_HEIGHT - 65;
    lcd.drawLine(10, y, SCREEN_WIDTH - 10, y, 0x4208);
    
    // Alternative: Manual list - größer
    y += 12;
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, y);
    lcd.print("Alle Beacons:");
    
    y += 20;
    drawButton(10, y, SCREEN_WIDTH - 20, BUTTON_HEIGHT, "Beacon-Liste anzeigen", COLOR_BUTTON);
}

void drawBeaconListScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Beacon-Liste", true);
    
    auto beacons = bleScanner.getBeacons();
    
    // Sort by RSSI (strongest first)
    std::sort(beacons.begin(), beacons.end(), [](const BeaconData& a, const BeaconData& b) {
        return a.rssi > b.rssi;
    });
    
    int y = HEADER_HEIGHT + 12;
    
    if (beacons.empty()) {
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_LARGE);
        lcd.setTextDatum(TC_DATUM);
        lcd.drawString("Keine Beacons", SCREEN_WIDTH / 2, y + 50);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.drawString("gefunden...", SCREEN_WIDTH / 2, y + 90);
    } else {
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.setCursor(10, y);
        lcd.printf("Gefunden: %u Beacon(s)", beacons.size());
        
        y += 25;
        
        int displayCount = 0;
        for (auto& beacon : beacons) {
            if (displayCount >= 6) break;  // Max 6 visible (größere Items)
            
            float dist = BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower);
            
            uint16_t bgColor = dist < 1.0 ? COLOR_SECONDARY : COLOR_BUTTON;
            int itemH = 42;
            lcd.fillRoundRect(10, y, SCREEN_WIDTH - 20, itemH, 6, bgColor);
            lcd.drawRoundRect(10, y, SCREEN_WIDTH - 20, itemH, 6, 0x4208);
            
            lcd.setTextColor(COLOR_BUTTON_TEXT);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(15, y + 6);
            
            // Always show MAC address
            lcd.printf("%s", beacon.macAddress.c_str());
            
            lcd.setCursor(15, y + 22);
            lcd.printf("%d dBm | %.2fm", beacon.rssi, dist);
            
            lcd.setCursor(15, y + 34);
            if (dist < 1.0) {
                lcd.setTextColor(COLOR_SECONDARY);
                lcd.print("< Tippen zum Zuordnen!");
            } else {
                lcd.setTextColor(TEXT_COLOR);
                lcd.print("zu weit");
            }
            
            y += itemH + BUTTON_MARGIN;
            displayCount++;
        }
    }
}

void drawRaceSetupScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Rennen einrichten", true);
    
    int y = HEADER_HEIGHT + 10;
    
    // Race Name Label - dunkel und klar
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, y);
    lcd.print("RENNEN-NAME");
    
    // Input field - größer
    int inputH = 45;
    lcd.fillRoundRect(10, y + 20, SCREEN_WIDTH - 20, inputH, 8, COLOR_BUTTON);
    lcd.drawRoundRect(10, y + 20, SCREEN_WIDTH - 20, inputH, 8, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(15, y + 33);
    if (uiState.raceName.length() > 0) {
        lcd.print(uiState.raceName);
    } else {
        lcd.setTextColor(0x630C);  // Darker gray placeholder
        lcd.print("Tippen...");
    }
    
    y += inputH + 20;
    
    // Duration with +/- Controls - Label dunkel
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, y);
    lcd.print("DAUER (MINUTEN)");
    
    y += 18;
    
    // Duration controls - BREITERE Buttons für bessere Lesbarkeit
    int btnSize = BUTTON_HEIGHT - 8;
    int btnWidth = 50;  // Breiter: 45 -> 50
    
    // "-5" Button (left) - breiter
    lcd.fillRoundRect(10, y, btnWidth, btnSize, 5, COLOR_BUTTON);
    lcd.drawRoundRect(10, y, btnWidth, btnSize, 5, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("-5", 10 + btnWidth/2, y + btnSize/2);
    
    // Value Display (center)
    lcd.fillRoundRect(65, y, 110, btnSize, 5, COLOR_SECONDARY);
    lcd.drawRoundRect(65, y, 110, btnSize, 5, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_LARGE);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(String(uiState.raceDuration), 120, y + btnSize/2);
    
    // "+5" Button (right) - breiter
    lcd.fillRoundRect(180, y, btnWidth, btnSize, 5, COLOR_BUTTON);
    lcd.drawRoundRect(180, y, btnWidth, btnSize, 5, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("+5", 180 + btnWidth/2, y + btnSize/2);
    
    // "+15" Button (far right) - breiter
    lcd.fillRoundRect(235, y, 50, btnSize, 5, COLOR_BUTTON);
    lcd.drawRoundRect(235, y, 50, btnSize, 5, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("+15", 260, y + btnSize/2);
    
    y += btnSize + 20;
    
    // Teams Label - dunkel
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(TL_DATUM);
    lcd.setCursor(10, y);
    
    int teamCount = lapCounter.getTeamCount();
    if (teamCount == 0) {
        lcd.print("KEINE TEAMS");
    } else {
        lcd.printf("TEAMS (%d)", teamCount);
    }
    
    y += 12;
    
    // Team list (compact)
    if (teamCount > 0) {
        for (uint8_t i = 1; i <= MAX_TEAMS && i <= teamCount; i++) {
            TeamData* team = lapCounter.getTeam(i);
            if (!team) continue;
            
            if (y > SCREEN_HEIGHT - 60) break;  // Space for Start button
            
            // Team box - größer
            uint16_t bgColor = uiState.selectedTeams[i-1] ? COLOR_SECONDARY : COLOR_BUTTON;
            int teamH = 28;
            lcd.fillRoundRect(10, y, SCREEN_WIDTH - 20, teamH, 5, bgColor);
            lcd.drawRoundRect(10, y, SCREEN_WIDTH - 20, teamH, 5, 0x4208);
            
            // Checkbox - größer
            lcd.fillCircle(25, y + teamH/2, 6, uiState.selectedTeams[i-1] ? 0x0000 : 0xFFFF);
            lcd.drawCircle(25, y + teamH/2, 6, 0x0000);
            
            // Team name - größer und dunkel
            lcd.setTextColor(COLOR_BUTTON_TEXT);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(38, y + 8);
            lcd.print(team->teamName);
            
            y += teamH + BUTTON_MARGIN;
        }
    }
    
    // Start button
    int btnY = SCREEN_HEIGHT - 50;
    uint16_t btnColor = (teamCount > 0) ? COLOR_SECONDARY : COLOR_BUTTON;
    drawButton(10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT, "RENNEN STARTEN", btnColor);
}

void drawRaceRunningScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    
    // Header with time - größer und klarer
    uint32_t elapsed = millis() - raceStartTime;
    uint32_t remaining = (raceDuration > elapsed) ? (raceDuration - elapsed) : 0;
    uint32_t minutes = remaining / 60000;
    uint32_t seconds = (remaining % 60000) / 1000;
    
    lcd.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_HEADER_BG);
    lcd.setTextColor(COLOR_HEADER_TEXT);
    lcd.setTextSize(TEXT_SIZE_LARGE);
    lcd.setTextDatum(MC_DATUM);
    char timeStr[10];
    sprintf(timeStr, "%02lu:%02lu", minutes, seconds);
    lcd.drawString("Zeit: " + String(timeStr), SCREEN_WIDTH / 2, HEADER_HEIGHT / 2);
    
    // Leaderboard - größere Items, besser lesbar
    auto leaderboard = lapCounter.getLeaderboard(true);
    
    int y = HEADER_HEIGHT + 15;
    int pos = 1;
    
    for (auto* team : leaderboard) {
        if (y > SCREEN_HEIGHT - 90) break;  // Platz für Buttons
        
        // Position & Name - größer
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.setCursor(10, y);
        lcd.printf("%u. %s", pos, team->teamName.c_str());
        
        // Laps - klar lesbar
        lcd.setCursor(240, y);
        uint16_t laps = team->lapCount > 0 ? team->lapCount - 1 : 0;
        lcd.printf("%uR", laps);
        
        // Last lap time - größer
        if (!team->laps.empty()) {
            LapTime& lastLap = team->laps.back();
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(20, y + 22);
            lcd.printf("Letzte: %lu.%03lu s", 
                      lastLap.duration / 1000, lastLap.duration % 1000);
        }
        
        y += 42;  // Mehr Abstand
        pos++;
    }
    
    // Buttons - größer
    int btnY = SCREEN_HEIGHT - 60;
    int btnW = (SCREEN_WIDTH - BUTTON_MARGIN * 3) / 2;
    drawButton(BUTTON_MARGIN, btnY, btnW, BUTTON_HEIGHT, "Pause", COLOR_WARNING);
    drawButton(BUTTON_MARGIN * 2 + btnW, btnY, btnW, BUTTON_HEIGHT, "Stop", COLOR_DANGER);
}

void drawRacePausedScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("RENNEN PAUSIERT", false);
    
    int y = HEADER_HEIGHT + 30;
    
    // PAUSE Text - sehr groß und dunkel
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_LARGE + 1);  // Size 4
    lcd.setTextDatum(TC_DATUM);
    lcd.drawString("PAUSE", SCREEN_WIDTH / 2, y);
    
    y += 50;
    
    // Time - größer
    uint32_t elapsed = millis() - raceStartTime;
    uint32_t minutes = elapsed / 60000;
    uint32_t seconds = (elapsed % 60000) / 1000;
    
    lcd.setTextSize(TEXT_SIZE_LARGE);
    lcd.setTextDatum(TC_DATUM);
    char timeStr[10];
    sprintf(timeStr, "%02lu:%02lu", minutes, seconds);
    lcd.drawString("Zeit: " + String(timeStr), SCREEN_WIDTH / 2, y);
    
    // Buttons - größer mit mehr Abstand
    y = SCREEN_HEIGHT - 120;
    drawButton(BUTTON_MARGIN, y, SCREEN_WIDTH - 2*BUTTON_MARGIN, 
              BUTTON_HEIGHT, "Weitermachen", COLOR_SECONDARY);
    
    y += BUTTON_HEIGHT + BUTTON_MARGIN;
    drawButton(BUTTON_MARGIN, y, SCREEN_WIDTH - 2*BUTTON_MARGIN, 
              BUTTON_HEIGHT, "Rennen beenden", COLOR_DANGER);
}

void drawRaceResultsScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Ergebnisse", true);
    
    // Show current race leaderboard (from memory)
    auto leaderboard = lapCounter.getLeaderboard(true);
    
    int y = HEADER_HEIGHT + 10;
    
    if (leaderboard.empty()) {
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_LARGE);
        lcd.setTextDatum(TC_DATUM);
        lcd.drawString("Keine Daten", SCREEN_WIDTH / 2, y + 60);
    } else {
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.setCursor(10, y);
        lcd.print("Aktuelles Rennen:");
        
        y += 20;
        
        int pos = 1;
        for (auto* team : leaderboard) {
            if (y > SCREEN_HEIGHT - BUTTON_HEIGHT - BUTTON_MARGIN) break;
            
            // Position & Name - größer
            lcd.setTextColor(TEXT_COLOR);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(10, y);
            lcd.printf("%u. %s", pos, team->teamName.c_str());
            
            // Laps - klar lesbar
            lcd.setCursor(240, y);
            uint16_t laps = team->lapCount > 0 ? team->lapCount - 1 : 0;
            lcd.printf("%uR", laps);
            
            // Best lap if available - größer
            if (team->bestLapDuration < UINT32_MAX) {
                lcd.setTextSize(TEXT_SIZE_NORMAL);
                lcd.setCursor(20, y + 22);
                lcd.printf("Beste: %lu.%03lu s", 
                          team->bestLapDuration / 1000, team->bestLapDuration % 1000);
            }
            
            y += LIST_ITEM_HEIGHT + BUTTON_MARGIN;
            pos++;
        }
    }
}

void drawSettingsScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Einstellungen", true);
    
    int y = HEADER_HEIGHT + 10;
    
    // === RSSI Thresholds (wichtigste Einstellung!) ===
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, y);
    lcd.print("LAP DETECTION:");
    
    y += 15;
    
    // RSSI NEAR (NAH)
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(1);
    lcd.setCursor(10, y);
    lcd.print("NAH:");
    
    y += 10;  // Label above, then controls
    
    int btnW = 40;
    int valW = 70;
    int centerX = SCREEN_WIDTH / 2;
    int totalW = btnW + 5 + valW + 5 + btnW;
    int startX = centerX - totalW / 2;
    
    // "-" Button
    drawButton(startX, y, btnW, 30, "-", COLOR_BUTTON);
    
    // Value Display
    lcd.fillRoundRect(startX + btnW + 5, y, valW, 30, 3, COLOR_SECONDARY);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(String(lapRssiNear), startX + btnW + 5 + valW/2, y + 15);
    lcd.setTextDatum(TL_DATUM);
    
    // "+" Button
    drawButton(startX + btnW + 5 + valW + 5, y, btnW, 30, "+", COLOR_BUTTON);
    
    y += 38;
    
    // RSSI FAR (WEG)
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(1);
    lcd.setCursor(10, y);
    lcd.print("WEG:");
    
    y += 10;  // Label above, then controls
    
    // "-" Button
    drawButton(startX, y, btnW, 30, "-", COLOR_BUTTON);
    
    // Value Display
    lcd.fillRoundRect(startX + btnW + 5, y, valW, 30, 3, COLOR_SECONDARY);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(String(lapRssiFar), startX + btnW + 5 + valW/2, y + 15);
    lcd.setTextDatum(TL_DATUM);
    
    // "+" Button
    drawButton(startX + btnW + 5 + valW + 5, y, btnW, 30, "+", COLOR_BUTTON);
    
    y += 38;
    
    // Separator
    lcd.drawLine(10, y, SCREEN_WIDTH - 10, y, 0x4208);
    y += 10;
    
    // === SD Card & Touch ===
    // SD Card Format/Init Button - BREIT GENUG für Text!
    int settingsBtnH = 30;  // Smaller than BUTTON_HEIGHT for better fit
    int settingsBtnW = 145;  // Breiter: 125 -> 145
    int settingsBtnX = SCREEN_WIDTH - settingsBtnW - 10;
    
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, y + 6);  // Center text vertically with smaller button
    if (dataLogger.isReady()) {
        lcd.print("SD-Format:");
        drawButton(settingsBtnX, y, settingsBtnW, settingsBtnH, "Formatieren", COLOR_WARNING);
    } else {
        lcd.print("SD-Karte:");
        drawButton(settingsBtnX, y, settingsBtnW, settingsBtnH, "Init/Format", COLOR_SECONDARY);
    }
    y += settingsBtnH + 8;
    
    // Touch Calibration Button
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, y + 6);
    lcd.print("Touch:");
    drawButton(settingsBtnX, y, settingsBtnW, settingsBtnH, "Kalibrieren", COLOR_SECONDARY);
}

// ============================================================
// Touch Handler
// ============================================================

void handleTouch(uint16_t x, uint16_t y) {
    // Back button in header - match exact button dimensions from drawHeader
    // Button is at (5, 5) with size (40, 30)
    // Only check if current screen has a back button (not HOME)
    if (uiState.currentScreen != SCREEN_HOME) {
        if (isTouchInRect(x, y, 5, 5, 40, 30)) {
            // Stop BLE scanning if on beacon assignment screens
            if (uiState.currentScreen == SCREEN_TEAM_BEACON_ASSIGN || 
                uiState.currentScreen == SCREEN_BEACON_LIST) {
                bleScanner.stopScan();
            }
            uiState.changeScreen(uiState.previousScreen);
            return;
        }
    }
    
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
        case SCREEN_SETTINGS:
            handleSettingsTouch(x, y);
            break;
        default:
            break;
    }
}

void handleHomeTouch(uint16_t x, uint16_t y) {
    // Match exact button dimensions from drawHomeScreen
    int btnHeight = 42;
    int btnSpacing = 8;
    int btnY = HEADER_HEIGHT + 10;
    int btnWidth = SCREEN_WIDTH - 2 * BUTTON_MARGIN;
    
    // Neues Rennen
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnWidth, btnHeight)) {
        uiState.changeScreen(SCREEN_RACE_SETUP);
        return;
    }
    btnY += btnHeight + btnSpacing;
    
    // Teams verwalten
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnWidth, btnHeight)) {
        uiState.changeScreen(SCREEN_TEAMS);
        return;
    }
    btnY += btnHeight + btnSpacing;
    
    // Ergebnisse
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnWidth, btnHeight)) {
        if (lapCounter.getTeamCount() > 0) {
            uiState.changeScreen(SCREEN_RACE_RESULTS);
        } else {
            showMessage("Info", "Keine Ergebnisse vorhanden", COLOR_WARNING);
        }
        return;
    }
    btnY += btnHeight + btnSpacing;
    
    // Einstellungen
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnWidth, btnHeight)) {
        uiState.changeScreen(SCREEN_SETTINGS);
        return;
    }
}

void handleTeamsTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, 5, 5, 60, HEADER_HEIGHT)) {
        uiState.changeScreen(SCREEN_HOME);
        return;
    }
    
    // Team items (can click to edit)
    auto teams = lapCounter.getAllTeams();
    int itemY = HEADER_HEIGHT + 10;
    int itemNum = 0;
    
    for (auto* team : teams) {
        if (itemNum >= 3) break;  // Max 3 visible
        
        if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 20, LIST_ITEM_HEIGHT)) {
            // Edit this team
            uiState.editingTeamId = team->teamId;
            uiState.editingTeamName = team->teamName;
            uiState.changeScreen(SCREEN_TEAM_EDIT);
            return;
        }
        
        itemY += LIST_ITEM_HEIGHT + 5;
        itemNum++;
    }
    
    // Add button
    int btnY = SCREEN_HEIGHT - 60;
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, BUTTON_HEIGHT)) {
        if (lapCounter.getTeamCount() >= MAX_TEAMS) {
            showMessage("Fehler", "Maximale Teams erreicht", COLOR_DANGER);
        } else {
            // Create new team with default name - find next FREE ID
            uint8_t newId = lapCounter.getNextFreeTeamId();
            if (newId == 0) {
                showMessage("Fehler", "Keine freie Team-ID", COLOR_DANGER);
                return;
            }
            
            String newName = "Team " + String(newId);
            String emptyBeacon = "";  // Beacon kann später zugeordnet werden
            
            if (lapCounter.addTeam(newId, newName, emptyBeacon)) {
                // Save to persistence (like old variant)
                persistence.saveTeams(lapCounter);
                
                uiState.editingTeamId = newId;
                uiState.editingTeamName = newName;
                uiState.changeScreen(SCREEN_TEAM_EDIT);
                
                Serial.printf("[Teams] Neues Team erstellt: ID=%u, Name=%s\n", newId, newName.c_str());
            } else {
                Serial.printf("[Teams] ERROR: Team konnte nicht erstellt werden (ID=%u)\n", newId);
                showMessage("Fehler", "Team konnte nicht erstellt werden", COLOR_DANGER);
            }
        }
        return;
    }
}

void handleTeamEditTouch(uint16_t x, uint16_t y) {
    // Back button - EXPLICIT handler (not global!)
    // Old variant uses SCREEN_WIDTH-60, drawHeader shows button at right
    if (isTouchInRect(x, y, SCREEN_WIDTH - 60, 0, 60, HEADER_HEIGHT)) {
        uiState.changeScreen(SCREEN_TEAMS);
        return;
    }
    
    int btnY = HEADER_HEIGHT + 15;
    
    // Team Name edit - USE KEYBOARD!
    if (isTouchInRect(x, y, 10, btnY + 25, SCREEN_WIDTH - 20, 36)) {
        String newName = inputText("Team-Name", uiState.editingTeamName);
        if (newName.length() > 0) {
            uiState.editingTeamName = newName;
        }
        return;
    }
    
    btnY += 70;
    
    // Beacon assignment
    if (isTouchInRect(x, y, 10, btnY + 25, SCREEN_WIDTH - 20, 36)) {
        uiState.previousScreen = SCREEN_TEAM_EDIT;
        uiState.changeScreen(SCREEN_TEAM_BEACON_ASSIGN);
        
        // Start BLE scanning
        if (!bleScanner.isScanning()) {
            bleScanner.startScan(0);
        }
        return;
    }
    
    btnY += 70;
    
    // Save button
    if (isTouchInRect(x, y, 10, btnY, (SCREEN_WIDTH - 30) / 2, BUTTON_HEIGHT)) {
        TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
        if (team) {
            team->teamName = uiState.editingTeamName;
            persistence.saveTeams(lapCounter);
            showMessage("Gespeichert", team->teamName, COLOR_SECONDARY);
            uiState.changeScreen(SCREEN_TEAMS);
        }
        return;
    }
    
    // Delete button
    if (isTouchInRect(x, y, (SCREEN_WIDTH + 10) / 2, btnY, (SCREEN_WIDTH - 30) / 2, BUTTON_HEIGHT)) {
        if (lapCounter.removeTeam(uiState.editingTeamId)) {
            persistence.saveTeams(lapCounter);
            showMessage("Geloscht", "Team wurde entfernt", COLOR_DANGER);
            uiState.changeScreen(SCREEN_TEAMS);
        }
        return;
    }
}

void handleBeaconAssignTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, 5, 5, 60, HEADER_HEIGHT)) {
        bleScanner.stopScan();
        uiState.changeScreen(uiState.previousScreen);
        return;
    }
    
    // Assign nearest beacon button (if visible and close enough)
    BeaconData* nearest = bleScanner.getNearestBeacon();
    if (nearest) {
        float dist = BLEScanner::rssiToDistance(nearest->rssi, nearest->txPower);
        
        if (dist < 1.0) {
            int btnY = HEADER_HEIGHT + 107;
            int btnW = (SCREEN_WIDTH - 30) / 2;
            
            // "Zuordnen" Button (links)
            if (isTouchInRect(x, y, 10, btnY, btnW, BUTTON_HEIGHT)) {
                TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
                if (team) {
                    team->beaconUUID = nearest->macAddress;
                    persistence.saveTeams(lapCounter);
                    showMessage("Zuordnung", "Beacon zugeordnet", COLOR_SECONDARY);
                    bleScanner.stopScan();
                    uiState.changeScreen(SCREEN_TEAM_EDIT);
                }
                return;
            }
            
            // "Liste" Button (rechts)
            if (isTouchInRect(x, y, 20 + btnW, btnY, btnW, BUTTON_HEIGHT)) {
                uiState.changeScreen(SCREEN_BEACON_LIST);
                return;
            }
        }
    }
    
    // Beacon list button (at bottom)
    int btnY = SCREEN_HEIGHT - 55;
    if (isTouchInRect(x, y, 10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT)) {
        uiState.changeScreen(SCREEN_BEACON_LIST);
        return;
    }
}

void handleBeaconListTouch(uint16_t x, uint16_t y) {
    // Back button - handled by global handler
    
    auto beacons = bleScanner.getBeacons();
    if (beacons.empty()) return;
    
    // Sort by RSSI (match draw order)
    std::sort(beacons.begin(), beacons.end(), [](const BeaconData& a, const BeaconData& b) {
        return a.rssi > b.rssi;
    });
    
    // Match exact layout from drawBeaconListScreen
    int y_start = HEADER_HEIGHT + 12;  // After header
    y_start += 25;  // After "Gefunden: X Beacon(s)" text
    
    int itemHeight = 42 + BUTTON_MARGIN;  // itemH + spacing (from drawBeaconListScreen)
    int itemH = 42;  // Individual item height
    
    int index = 0;
    for (auto& beacon : beacons) {
        if (index >= 6) break;  // Max 6 visible (match drawBeaconListScreen)
        
        int itemY = y_start + (index * itemHeight);
        
        // Touch area for this beacon (full item height)
        if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 20, itemH)) {
            Serial.printf("[Touch] Beacon #%d clicked: MAC=%s UUID=%s (RSSI=%d) at y=%d\n", 
                         index, beacon.macAddress.c_str(), beacon.uuid.c_str(), beacon.rssi, itemY);
            
            float dist = BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower);
            
            if (dist < 1.0) {
                // Assign this beacon to current team
                TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
                if (team) {
                    // IMPORTANT: Store MAC address, not UUID!
                    team->beaconUUID = beacon.macAddress;
                    persistence.saveTeams(lapCounter);
                    
                    Serial.printf("[Team %u] Beacon assigned: MAC=%s (UUID=%s)\n", 
                                 uiState.editingTeamId, beacon.macAddress.c_str(), beacon.uuid.c_str());
                    
                    showMessage("Zugeordnet", "Beacon erfolgreich zugeordnet", COLOR_SECONDARY);
                    bleScanner.stopScan();
                    uiState.changeScreen(SCREEN_TEAM_EDIT);
                    return;
                }
            } else {
                showMessage("Hinweis", "Beacon zu weit! Naher halten (<1m)", COLOR_WARNING);
            }
            return;
        }
        
        index++;
    }
}

void handleRaceSetupTouch(uint16_t x, uint16_t y) {
    // Back button handled by global handler
    
    int yPos = HEADER_HEIGHT + 10;
    
    // Race Name input - USE KEYBOARD!
    int inputH = 45;
    // MATCH drawRaceSetupScreen: label at y, input at y+20
    if (isTouchInRect(x, y, 10, yPos + 20, SCREEN_WIDTH - 20, inputH)) {
        String newName = inputText("Rennen-Name", uiState.raceName);
        if (newName.length() > 0) {
            uiState.raceName = newName;
        }
        return;
    }
    
    // MATCH drawRaceSetupScreen: yPos += inputH + 20, then +18 for duration label
    yPos += inputH + 20;  // After input box
    yPos += 18;  // After "DAUER (MINUTEN)" label
    
    // Duration controls - EXACT positions from drawRaceSetupScreen
    int btnSize = BUTTON_HEIGHT - 8;  // Should be 34
    int btnWidth = 50;  // Width for +5 and -5 buttons
    
    // "-5" Button (left) - x=10, w=50
    if (isTouchInRect(x, y, 10, yPos, 50, btnSize)) {
        if (uiState.raceDuration >= 5) {
            uiState.raceDuration -= 5;
        } else {
            uiState.raceDuration = 1;
        }
        Serial.printf("[RaceSetup] Duration: %u min (-5)\n", uiState.raceDuration);
        uiState.needsRedraw = true;
        return;
    }
    
    // Time display (center) - x=65, w=110 - CLICKABLE for manual input
    if (isTouchInRect(x, y, 65, yPos, 110, btnSize)) {
        // TODO: Manual time input (keyboard)
        showMessage("Info", "Manuelle Eingabe kommt noch", COLOR_WARNING);
        return;
    }
    
    // "+5" Button (right) - x=180, w=50
    if (isTouchInRect(x, y, 180, yPos, 50, btnSize)) {
        if (uiState.raceDuration <= 175) {
            uiState.raceDuration += 5;
        } else {
            uiState.raceDuration = 180;
        }
        Serial.printf("[RaceSetup] Duration: %u min (+5)\n", uiState.raceDuration);
        uiState.needsRedraw = true;
        return;
    }
    
    // "+15" Button (far right) - x=235, w=50
    if (isTouchInRect(x, y, 235, yPos, 50, btnSize)) {
        if (uiState.raceDuration <= 165) {
            uiState.raceDuration += 15;
        } else {
            uiState.raceDuration = 180;
        }
        Serial.printf("[RaceSetup] Duration: %u min (+15)\n", uiState.raceDuration);
        uiState.needsRedraw = true;
        return;
    }
    
    yPos += btnSize + 20 + 12;  // btnSize + spacing + teams label
    
    // Team checkboxes - iterate through all teams properly
    int teamCount = lapCounter.getTeamCount();
    if (teamCount > 0) {
        auto teams = lapCounter.getAllTeams();
        for (auto* team : teams) {
            if (!team) continue;
            if (yPos > SCREEN_HEIGHT - 60) break;  // Space for Start button
            
            int teamH = 28;
            if (isTouchInRect(x, y, 10, yPos, SCREEN_WIDTH - 20, teamH)) {
                // Toggle selection for this team ID (index = teamId - 1)
                uiState.selectedTeams[team->teamId - 1] = !uiState.selectedTeams[team->teamId - 1];
                Serial.printf("[RaceSetup] Team %u (%s) %s\n", 
                             team->teamId, team->teamName.c_str(), 
                             uiState.selectedTeams[team->teamId - 1] ? "selected" : "deselected");
                uiState.needsRedraw = true;
                return;
            }
            
            yPos += teamH + 3;  // Match drawRaceSetupScreen spacing
        }
    }
    
    // Start button
    int btnY = SCREEN_HEIGHT - 50;
    if (isTouchInRect(x, y, 10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT)) {
        if (lapCounter.getTeamCount() == 0) {
            showMessage("Fehler", "Keine Teams!", COLOR_DANGER);
            return;
        }
        
        // Start race
        currentRaceName = uiState.raceName.length() > 0 ? uiState.raceName : "Rennen";
        raceDuration = uiState.raceDuration * 60 * 1000;  // Convert to ms
        raceStartTime = millis();
        raceRunning = true;
        
        // Save race config
        persistence.saveConfig(currentRaceName, uiState.raceDuration);
        
        // Start data logging
        if (dataLogger.isReady()) {
            dataLogger.startNewRace(currentRaceName);
        }
        
        // Start BLE scanning
        if (!bleScanner.isScanning()) {
            bleScanner.startScan(0);
        }
        
        // Reset all teams
        lapCounter.reset();
        
        // Reset beacon presence
        beaconPresence.clear();
        
        uiState.changeScreen(SCREEN_RACE_RUNNING);
        
        Serial.println("\n=== RACE STARTED ===");
        Serial.printf("Name: %s\n", currentRaceName.c_str());
        Serial.printf("Duration: %lu minutes\n", uiState.raceDuration);
        Serial.printf("Teams: %u\n", lapCounter.getTeamCount());
    }
}

void handleRaceRunningTouch(uint16_t x, uint16_t y) {
    int btnY = SCREEN_HEIGHT - 60;
    int btnW = (SCREEN_WIDTH - BUTTON_MARGIN * 3) / 2;
    
    // Pause button
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, btnW, BUTTON_HEIGHT)) {
        raceRunning = false;
        uiState.changeScreen(SCREEN_RACE_PAUSED);
        Serial.println("\n=== RACE PAUSED ===");
        return;
    }
    
    // Stop button
    if (isTouchInRect(x, y, BUTTON_MARGIN * 2 + btnW, btnY, btnW, BUTTON_HEIGHT)) {
        raceRunning = false;
        uiState.changeScreen(SCREEN_RACE_RESULTS);
        
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
        uiState.changeScreen(SCREEN_RACE_RUNNING);
        
        if (!bleScanner.isScanning()) {
            bleScanner.startScan(0);
        }
        
        Serial.println("\n=== RACE RESUMED ===");
        return;
    }
    
    btnY += BUTTON_HEIGHT + 10;
    
    // End race button
    if (isTouchInRect(x, y, BUTTON_MARGIN, btnY, SCREEN_WIDTH - 2*BUTTON_MARGIN, BUTTON_HEIGHT)) {
        raceRunning = false;
        uiState.changeScreen(SCREEN_RACE_RESULTS);
        
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
    if (isTouchInRect(x, y, 5, 5, 60, HEADER_HEIGHT)) {
        uiState.changeScreen(SCREEN_HOME);
        return;
    }
}

void handleSettingsTouch(uint16_t x, uint16_t y) {
    // Back button handled by global handler
    
    int yStart = HEADER_HEIGHT + 10 + 15 + 10;  // Title + "LAP DETECTION:" + "NAH:" label
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
            persistence.saveRssiThresholds(lapRssiNear, lapRssiFar);
            
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
            persistence.saveRssiThresholds(lapRssiNear, lapRssiFar);
            
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
            persistence.saveRssiThresholds(lapRssiNear, lapRssiFar);
            
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
            persistence.saveRssiThresholds(lapRssiNear, lapRssiFar);
            
            Serial.printf("[Settings] RSSI FAR: %d dBm\n", lapRssiFar);
            uiState.needsRedraw = true;
        }
        return;
    }
    
    // === SD & Touch Buttons ===
    int btnY = yFar + 38 + 10;  // FAR controls + separator
    int settingsBtnH = 30;  // Match drawSettingsScreen
    int settingsBtnW = 145;  // Match drawSettingsScreen
    int settingsBtnX = SCREEN_WIDTH - settingsBtnW - 10;
    
    // SD Format/Init button - match exact position from draw
    if (isTouchInRect(x, y, settingsBtnX, btnY, settingsBtnW, settingsBtnH)) {
        if (dataLogger.isReady()) {
            // Confirmation dialog
            showMessage("WARNUNG!", "Alle Daten werden geloscht!", COLOR_DANGER);
            delay(2000);
            // TODO: Add proper confirmation button instead of auto-continue
            if (dataLogger.formatSD()) {
                showMessage("Erfolg", "SD-Karte formatiert", COLOR_SECONDARY);
                // Reinitialize SD after format
                dataLogger.begin(SD_CS_PIN);
            } else {
                showMessage("Fehler", "Formatierung fehlgeschlagen", COLOR_DANGER);
            }
        } else {
            // Try to initialize and format
            if (dataLogger.begin(SD_CS_PIN)) {
                showMessage("Erfolg", "SD-Karte initialisiert", COLOR_SECONDARY);
            } else {
                showMessage("Fehler", "SD-Init fehlgeschlagen", COLOR_DANGER);
            }
        }
        uiState.needsRedraw = true;
        return;
    }
    btnY += settingsBtnH + 8;  // Match spacing from drawSettingsScreen
    
    // Touch Calibration button - match exact position from draw
    if (isTouchInRect(x, y, settingsBtnX, btnY, settingsBtnW, settingsBtnH)) {
        display.calibrateTouch();
        uiState.needsRedraw = true;
        return;
    }
}

