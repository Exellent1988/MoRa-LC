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

// Helper function to check if a beacon is already assigned to a team
static bool isBeaconAssigned(const String& macAddress) {
    auto teams = lapCounter.getAllTeams();
    for (auto* team : teams) {
        if (team && team->beaconUUID.length() > 0 && team->beaconUUID == macAddress) {
            return true;
        }
    }
    return false;
}

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
    lcd.setTextSize(1);  // Smaller text for footer (intentional - space constraint)
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
        // Calculate max visible teams (reserve space for + button: 35px)
        int availableHeight = SCREEN_HEIGHT - 35 - BUTTON_MARGIN - y;
        int maxVisible = availableHeight / (LIST_ITEM_HEIGHT + BUTTON_MARGIN);
        
        // Ensure at least 3 visible
        if (maxVisible < 3) maxVisible = 3;
        
        bool needsScroll = (int)teams.size() > maxVisible;
        
        // Up arrow (ganz oben, klein)
        if (needsScroll && uiState.scrollOffset > 0) {
            lcd.fillTriangle(SCREEN_WIDTH - 25, y + 2, 
                            SCREEN_WIDTH - 15, y + 12, 
                            SCREEN_WIDTH - 35, y + 12, 
                            COLOR_SECONDARY);
        }
        
        // Display teams with scroll offset
        int displayCount = 0;
        int startIndex = uiState.scrollOffset;
        
        for (size_t i = startIndex; i < teams.size() && displayCount < maxVisible; i++) {
            auto* team = teams[i];
            
            // Team Item - kompakt aber lesbar
            lcd.fillRoundRect(10, y, SCREEN_WIDTH - 45, LIST_ITEM_HEIGHT, 5, COLOR_BUTTON);
            lcd.drawRoundRect(10, y, SCREEN_WIDTH - 45, LIST_ITEM_HEIGHT, 5, 0x4208);
            
            // Team Name
            lcd.setTextColor(COLOR_BUTTON_TEXT);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(15, y + 6);
            lcd.printf("%u. %s", team->teamId, team->teamName.c_str());
            
            // Beacon Info
            lcd.setTextSize(1);  // Smaller for secondary info (intentional)
            lcd.setCursor(15, y + 26);
            if (team->beaconUUID.length() > 0) {
                lcd.printf("Beacon: %s", formatBeaconMAC(team->beaconUUID).c_str());
            } else {
                lcd.print("Kein Beacon");
            }
            
            y += LIST_ITEM_HEIGHT + BUTTON_MARGIN;
            displayCount++;
        }
        
        // Down arrow (ganz unten, klein)
        if (needsScroll && startIndex + displayCount < (int)teams.size()) {
            lcd.fillTriangle(SCREEN_WIDTH - 25, y - 2, 
                            SCREEN_WIDTH - 15, y - 12, 
                            SCREEN_WIDTH - 35, y - 12, 
                            COLOR_SECONDARY);
        }
    }
    
    // Add button - KLEIN, nur "+" Icon, ganz unten links
    int btnSize = 35;  // Kompakt: 35x35px
    int btnY = SCREEN_HEIGHT - btnSize;  // Ganz unten, kein Rand
    lcd.fillRoundRect(2, btnY, btnSize, btnSize, 8, COLOR_SECONDARY);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(3);  // Groß für "+"
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("+", 2 + btnSize/2, btnY + btnSize/2);
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
    bool hasBeacon = false;
    if (team && team->beaconUUID.length() > 0) {
        beaconText = formatBeaconMAC(team->beaconUUID);
        hasBeacon = true;
    }
    
    drawButton(10, y + 25, SCREEN_WIDTH - 20, 36, beaconText, COLOR_BUTTON);
    
    y += 70;
    
    // Remove beacon assignment button (only if beacon is assigned)
    if (hasBeacon) {
        drawButton(10, y, SCREEN_WIDTH - 20, BUTTON_HEIGHT, "Zuordnung entfernen", COLOR_WARNING);
        y += BUTTON_HEIGHT + BUTTON_MARGIN;
    }
    
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
    
    // Nearest Beacon - größer (only show if not already assigned)
    BeaconData* nearest = nullptr;
    auto allBeacons = bleScanner.getBeacons();
    for (auto& beacon : allBeacons) {
        if (!isBeaconAssigned(beacon.macAddress)) {
            if (!nearest || beacon.rssi > nearest->rssi) {
                nearest = &beacon;
            }
        }
    }
    
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
        lcd.printf("MAC: %s", formatBeaconMAC(nearest->macAddress).c_str());
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
        
        // Calculate max visible beacons (reserve space for scroll arrows)
        int availableHeight = SCREEN_HEIGHT - y - BUTTON_MARGIN;
        int itemH = 42;
        int itemSpacing = BUTTON_MARGIN;
        int maxVisible = availableHeight / (itemH + itemSpacing);
        
        // Ensure at least 3 visible
        if (maxVisible < 3) maxVisible = 3;
        
        bool needsScroll = (int)beacons.size() > maxVisible;
        
        // Up arrow (ganz oben, rechts)
        if (needsScroll && uiState.scrollOffset > 0) {
            lcd.fillTriangle(SCREEN_WIDTH - 25, y + 2, 
                            SCREEN_WIDTH - 15, y + 12, 
                            SCREEN_WIDTH - 35, y + 12, 
                            COLOR_SECONDARY);
        }
        
        // Filter out already assigned beacons
        std::vector<BeaconData> unassignedBeacons;
        for (auto& beacon : beacons) {
            if (!isBeaconAssigned(beacon.macAddress)) {
                unassignedBeacons.push_back(beacon);
            }
        }
        
        // Update needsScroll based on filtered list
        needsScroll = (int)unassignedBeacons.size() > maxVisible;
        
        // Display beacons with scroll offset (only unassigned)
        int displayCount = 0;
        int startIndex = uiState.scrollOffset;
        
        for (size_t i = startIndex; i < unassignedBeacons.size() && displayCount < maxVisible; i++) {
            auto& beacon = unassignedBeacons[i];
            
            float dist = BLEScanner::rssiToDistance(beacon.rssi, beacon.txPower);
            
            uint16_t bgColor = dist < 1.0 ? COLOR_SECONDARY : COLOR_BUTTON;
            lcd.fillRoundRect(10, y, SCREEN_WIDTH - 45, itemH, 6, bgColor);
            lcd.drawRoundRect(10, y, SCREEN_WIDTH - 45, itemH, 6, 0x4208);
            
            lcd.setTextColor(COLOR_BUTTON_TEXT);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(15, y + 6);
            
            // Always show MAC address (formatted: only last 4 digits for c3:00:00:XX:XX:XX)
            lcd.printf("%s", formatBeaconMAC(beacon.macAddress).c_str());
            
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
            
            y += itemH + itemSpacing;
            displayCount++;
        }
        
        // Down arrow (ganz unten, rechts)
        if (needsScroll && startIndex + displayCount < (int)unassignedBeacons.size()) {
            lcd.fillTriangle(SCREEN_WIDTH - 25, y - 2, 
                            SCREEN_WIDTH - 15, y - 12, 
                            SCREEN_WIDTH - 35, y - 12, 
                            COLOR_SECONDARY);
        }
    }
}

void drawRaceSetupScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Rennen einrichten", true);
    
    int y = HEADER_HEIGHT + 8;
    
    // Race Name Label - kompakter
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(1);  // Smaller label (intentional - space constraint)
    lcd.setCursor(10, y);
    lcd.print("RENNEN-NAME");
    
    // Input field - kompakter
    int inputH = 35;
    lcd.fillRoundRect(10, y + 12, SCREEN_WIDTH - 20, inputH, 5, COLOR_BUTTON);
    lcd.drawRoundRect(10, y + 12, SCREEN_WIDTH - 20, inputH, 5, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(15, y + 25);
    String displayName = uiState.raceName;
    if (displayName.length() > 20) {
        displayName = displayName.substring(0, 17) + "...";
    }
    if (displayName.length() > 0) {
        lcd.print(displayName);
    } else {
        lcd.setTextColor(0x630C);  // Darker gray placeholder
        lcd.print("Tippen...");
    }
    
    y += inputH + 12;
    
    // Duration with +/- Controls - kompakter
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(1);  // Smaller label (intentional - space constraint)
    lcd.setCursor(10, y);
    lcd.print("DAUER (MIN)");
    
    y += 12;
    
    // Duration controls - kompakter
    int btnSize = 28;
    int btnWidth = 45;
    
    // "-5" Button (left)
    lcd.fillRoundRect(10, y, btnWidth, btnSize, 4, COLOR_BUTTON);
    lcd.drawRoundRect(10, y, btnWidth, btnSize, 4, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("-5", 10 + btnWidth/2, y + btnSize/2);
    
    // Value Display (center)
    lcd.fillRoundRect(60, y, 100, btnSize, 4, COLOR_SECONDARY);
    lcd.drawRoundRect(60, y, 100, btnSize, 4, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_LARGE);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString(String(uiState.raceDuration), 110, y + btnSize/2);
    
    // "+5" Button (right)
    lcd.fillRoundRect(165, y, btnWidth, btnSize, 4, COLOR_BUTTON);
    lcd.drawRoundRect(165, y, btnWidth, btnSize, 4, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("+5", 165 + btnWidth/2, y + btnSize/2);
    
    // "+15" Button (far right)
    lcd.fillRoundRect(215, y, 45, btnSize, 4, COLOR_BUTTON);
    lcd.drawRoundRect(215, y, 45, btnSize, 4, 0x4208);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("+15", 237, y + btnSize/2);
    
    y += btnSize + 10;
    
    // Teams Label - kompakter
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(1);  // Smaller label (intentional - space constraint)
    lcd.setCursor(10, y);
    
    int teamCount = lapCounter.getTeamCount();
    if (teamCount == 0) {
        lcd.print("KEINE TEAMS");
    } else {
        lcd.printf("TEAMS (%d)", teamCount);
    }
    
    y += 10;
    
    // Fixed start button position (always at bottom)
    int startBtnY = SCREEN_HEIGHT - BUTTON_HEIGHT - 4;
    int teamListHeight = startBtnY - y - 8;  // Available height for team list
    
    // Team list with scroll (scrollable if needed)
    if (teamCount > 0) {
        auto teams = lapCounter.getAllTeams();
        
        // Calculate max visible teams
        int teamH = 26;
        int teamSpacing = 4;
        int maxVisible = teamListHeight / (teamH + teamSpacing);
        if (maxVisible < 1) maxVisible = 1;
        
        bool needsScroll = (int)teams.size() > maxVisible;
        
        // Up arrow (if scrolling)
        if (needsScroll && uiState.raceSetupScrollOffset > 0) {
            lcd.fillTriangle(SCREEN_WIDTH - 20, y + 2, 
                            SCREEN_WIDTH - 12, y + 10, 
                            SCREEN_WIDTH - 28, y + 10, 
                            COLOR_SECONDARY);
        }
        
        // Display teams with scroll offset
        int displayCount = 0;
        int startIndex = uiState.raceSetupScrollOffset;
        
        for (size_t i = startIndex; i < teams.size() && displayCount < maxVisible; i++) {
            TeamData* team = teams[i];
            if (!team) continue;
            
            uint16_t bgColor = uiState.selectedTeams[team->teamId - 1] ? COLOR_SECONDARY : COLOR_BUTTON;
            lcd.fillRoundRect(10, y, SCREEN_WIDTH - 35, teamH, 4, bgColor);
            lcd.drawRoundRect(10, y, SCREEN_WIDTH - 35, teamH, 4, 0x4208);
            
            // Checkbox
            lcd.fillCircle(22, y + teamH/2, 5, uiState.selectedTeams[team->teamId - 1] ? 0x0000 : 0xFFFF);
            lcd.drawCircle(22, y + teamH/2, 5, 0x0000);
            
            // Team name
            lcd.setTextColor(COLOR_BUTTON_TEXT);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(32, y + 6);
            String teamDisplayName = team->teamName;
            if (teamDisplayName.length() > 18) {
                teamDisplayName = teamDisplayName.substring(0, 15) + "...";
            }
            lcd.print(teamDisplayName);
            
            y += teamH + teamSpacing;
            displayCount++;
        }
        
        // Down arrow (if scrolling)
        if (needsScroll && startIndex + displayCount < (int)teams.size()) {
            lcd.fillTriangle(SCREEN_WIDTH - 20, y - 2, 
                            SCREEN_WIDTH - 12, y - 10, 
                            SCREEN_WIDTH - 28, y - 10, 
                            COLOR_SECONDARY);
        }
    }
    
    // Start button - FIXED at bottom
    uint16_t btnColor = (teamCount > 0) ? COLOR_SECONDARY : COLOR_BUTTON;
    drawButton(10, startBtnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT, "RENNEN STARTEN", btnColor);
}

void drawRaceCountdownScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    
    // Calculate countdown (10 seconds)
    extern uint32_t countdownStartTime;
    uint32_t elapsed = millis() - countdownStartTime;
    uint32_t remaining = (elapsed < 10000) ? (10000 - elapsed) : 0;
    uint32_t seconds = remaining / 1000;
    
    // Large countdown number in center
    lcd.setTextColor(COLOR_SECONDARY);
    lcd.setTextSize(6);  // Very large
    lcd.setTextDatum(MC_DATUM);
    
    if (seconds > 0) {
        char countStr[4];
        sprintf(countStr, "%lu", seconds);
        lcd.drawString(countStr, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20);
    } else {
        lcd.drawString("GO!", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20);
    }
    
    // Race name
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(TC_DATUM);
    lcd.drawString(currentRaceName, SCREEN_WIDTH / 2, 50);
    
    // Info text
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setTextDatum(TC_DATUM);
    lcd.drawString("Rennen startet...", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 60);
}

// Helper function to update only time and lap counts (partial update)
void updateRaceRunningScreenPartial() {
    LGFX& lcd = display.getDisplay();
    
    // Update header time only (clear and redraw)
    lcd.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_HEADER_BG);
    uint32_t elapsed = millis() - raceStartTime;
    uint32_t remaining = (raceDuration > elapsed) ? (raceDuration - elapsed) : 0;
    uint32_t minutes = remaining / 60000;
    uint32_t seconds = (remaining % 60000) / 1000;
    
    lcd.setTextColor(COLOR_HEADER_TEXT);
    lcd.setTextSize(TEXT_SIZE_LARGE);
    lcd.setTextDatum(MC_DATUM);
    char timeStr[10];
    sprintf(timeStr, "%02lu:%02lu", minutes, seconds);
    lcd.drawString("Zeit: " + String(timeStr), SCREEN_WIDTH / 2, HEADER_HEIGHT / 2);
    
    // Update leaderboard items (only laps and times, not names/positions)
    auto leaderboard = lapCounter.getLeaderboard(true);
    int y = HEADER_HEIGHT + 12;
    int itemH = 42;
    int itemSpacing = 4;
    int btnAreaHeight = 50;
    int availableHeight = SCREEN_HEIGHT - y - btnAreaHeight;
    int maxVisible = availableHeight / (itemH + itemSpacing);
    if (maxVisible < 2) maxVisible = 2;
    
    int displayCount = 0;
    int startIndex = uiState.raceRunningScrollOffset;
    
    for (size_t i = startIndex; i < leaderboard.size() && displayCount < maxVisible; i++) {
        auto* team = leaderboard[i];
        
        // Clear old lap count area (right side)
        lcd.fillRect(240, y, 70, 22, BACKGROUND_COLOR);
        
        // Update lap count
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.setCursor(240, y);
        uint16_t laps = team->lapCount > 0 ? team->lapCount - 1 : 0;
        lcd.printf("%uR", laps);
        
        // Clear old lap time area
        lcd.fillRect(20, y + 22, 200, 20, BACKGROUND_COLOR);
        
        // Update last lap time
        if (!team->laps.empty()) {
            LapTime& lastLap = team->laps.back();
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(20, y + 22);
            lcd.printf("Letzte: %lu.%03lu s", 
                      lastLap.duration / 1000, lastLap.duration % 1000);
        }
        
        y += itemH + itemSpacing;
        displayCount++;
    }
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
    
    // Leaderboard - scrollable with auto-scroll
    auto leaderboard = lapCounter.getLeaderboard(true);
    
    int y = HEADER_HEIGHT + 12;
    
    // Calculate max visible teams (reserve space for buttons)
    int btnAreaHeight = 50;
    int availableHeight = SCREEN_HEIGHT - y - btnAreaHeight;
    int itemH = 42;
    int itemSpacing = 4;
    int maxVisible = availableHeight / (itemH + itemSpacing);
    if (maxVisible < 2) maxVisible = 2;
    
    bool needsScroll = (int)leaderboard.size() > maxVisible;
    
    // Display teams with scroll offset
    int displayCount = 0;
    int startIndex = uiState.raceRunningScrollOffset;
    int pos = startIndex + 1;
    
    for (size_t i = startIndex; i < leaderboard.size() && displayCount < maxVisible; i++) {
        auto* team = leaderboard[i];
        
        // Position & Name - größer
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.setCursor(10, y);
        String teamDisplayName = team->teamName;
        if (teamDisplayName.length() > 15) {
            teamDisplayName = teamDisplayName.substring(0, 12) + "...";
        }
        lcd.printf("%u. %s", pos, teamDisplayName.c_str());
        
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
        
        y += itemH + itemSpacing;
        pos++;
        displayCount++;
    }
    
    // Buttons - KLEIN in Ecken, ganz am Rand mit Icons
    int btnSize = 40;  // Kompakter: 40x40px
    int btnY = SCREEN_HEIGHT - btnSize - 2;  // Näher am Rand: 2px
    
    // Pause button (links unten) - Icon: ||
    lcd.fillRoundRect(2, btnY, btnSize, btnSize, 6, COLOR_WARNING);
    lcd.setTextColor(COLOR_BUTTON_TEXT);
    lcd.setTextSize(2);  // Mittel für Icon
    lcd.setTextDatum(MC_DATUM);
    lcd.drawString("||", 2 + btnSize/2, btnY + btnSize/2);
    
    // Stop button (rechts unten) - Icon: ◼
    lcd.fillRoundRect(SCREEN_WIDTH - btnSize - 2, btnY, btnSize, btnSize, 6, COLOR_DANGER);
    lcd.fillRect(SCREEN_WIDTH - btnSize - 2 + 12, btnY + 12, 16, 16, COLOR_BUTTON_TEXT);
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
        
        y += 18;
        
        // Calculate max visible teams
        int availableHeight = SCREEN_HEIGHT - y - BUTTON_MARGIN;
        int itemH = LIST_ITEM_HEIGHT;
        int itemSpacing = BUTTON_MARGIN;
        int maxVisible = availableHeight / (itemH + itemSpacing);
        if (maxVisible < 2) maxVisible = 2;
        
        bool needsScroll = (int)leaderboard.size() > maxVisible;
        
        // Up arrow (if scrolling)
        if (needsScroll && uiState.resultsScrollOffset > 0) {
            lcd.fillTriangle(SCREEN_WIDTH - 25, y + 2, 
                            SCREEN_WIDTH - 15, y + 12, 
                            SCREEN_WIDTH - 35, y + 12, 
                            COLOR_SECONDARY);
        }
        
        // Display teams with scroll offset
        int displayCount = 0;
        int startIndex = uiState.resultsScrollOffset;
        int pos = startIndex + 1;
        
        for (size_t i = startIndex; i < leaderboard.size() && displayCount < maxVisible; i++) {
            auto* team = leaderboard[i];
            
            // Position & Name - größer
            lcd.setTextColor(TEXT_COLOR);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(10, y);
            String teamDisplayName = team->teamName;
            if (teamDisplayName.length() > 15) {
                teamDisplayName = teamDisplayName.substring(0, 12) + "...";
            }
            lcd.printf("%u. %s", pos, teamDisplayName.c_str());
            
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
            
            y += itemH + itemSpacing;
            pos++;
            displayCount++;
        }
        
        // Down arrow (if scrolling)
        if (needsScroll && startIndex + displayCount < (int)leaderboard.size()) {
            lcd.fillTriangle(SCREEN_WIDTH - 25, y - 2, 
                            SCREEN_WIDTH - 15, y - 12, 
                            SCREEN_WIDTH - 35, y - 12, 
                            COLOR_SECONDARY);
        }
    }
    
    // Button for old results (if SD card is ready)
    if (dataLogger.isReady()) {
        int btnY = SCREEN_HEIGHT - BUTTON_HEIGHT - 4;
        drawButton(10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT, "Alte Ergebnisse", COLOR_BUTTON);
    }
}

void drawOldResultsListScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Alte Ergebnisse", true);
    
    int y = HEADER_HEIGHT + 10;
    
    // Get list of race files
    String raceFiles = dataLogger.getRaceFileList(20);
    
    if (raceFiles.length() == 0) {
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_LARGE);
        lcd.setTextDatum(TC_DATUM);
        lcd.drawString("Keine alten", SCREEN_WIDTH / 2, y + 50);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.drawString("Rennen gefunden", SCREEN_WIDTH / 2, y + 90);
    } else {
        // Parse race files into array
        std::vector<String> files;
        int startPos = 0;
        int newlinePos = raceFiles.indexOf('\n', startPos);
        
        while (newlinePos >= 0) {
            String filename = raceFiles.substring(startPos, newlinePos);
            if (filename.length() > 0) {
                files.push_back("/races/" + filename);
            }
            startPos = newlinePos + 1;
            newlinePos = raceFiles.indexOf('\n', startPos);
        }
        
        // Last file (if no newline at end)
        if (startPos < raceFiles.length()) {
            String filename = raceFiles.substring(startPos);
            if (filename.length() > 0) {
                files.push_back("/races/" + filename);
            }
        }
        
        // Calculate max visible files
        int availableHeight = SCREEN_HEIGHT - y - BUTTON_MARGIN;
        int itemH = 30;
        int itemSpacing = 4;
        int maxVisible = availableHeight / (itemH + itemSpacing);
        if (maxVisible < 2) maxVisible = 2;
        
        bool needsScroll = (int)files.size() > maxVisible;
        
        // Up arrow (if scrolling)
        if (needsScroll && uiState.oldResultsListScrollOffset > 0) {
            lcd.fillTriangle(SCREEN_WIDTH - 25, y + 2, 
                            SCREEN_WIDTH - 15, y + 12, 
                            SCREEN_WIDTH - 35, y + 12, 
                            COLOR_SECONDARY);
        }
        
        // Display files with scroll offset
        int displayCount = 0;
        int startIndex = uiState.oldResultsListScrollOffset;
        
        for (size_t i = startIndex; i < files.size() && displayCount < maxVisible; i++) {
            String filename = files[i];
            // Extract just the filename (without path)
            String displayName = filename;
            if (displayName.startsWith("/races/")) {
                displayName = displayName.substring(7);
            }
            // Truncate if too long
            if (displayName.length() > 25) {
                displayName = displayName.substring(0, 22) + "...";
            }
            
            lcd.fillRoundRect(10, y, SCREEN_WIDTH - 35, itemH, 4, COLOR_BUTTON);
            lcd.drawRoundRect(10, y, SCREEN_WIDTH - 35, itemH, 4, 0x4208);
            
            lcd.setTextColor(COLOR_BUTTON_TEXT);
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(15, y + 8);
            lcd.print(displayName);
            
            y += itemH + itemSpacing;
            displayCount++;
        }
        
        // Down arrow (if scrolling)
        if (needsScroll && startIndex + displayCount < (int)files.size()) {
            lcd.fillTriangle(SCREEN_WIDTH - 25, y - 2, 
                            SCREEN_WIDTH - 15, y - 12, 
                            SCREEN_WIDTH - 35, y - 12, 
                            COLOR_SECONDARY);
        }
    }
}

void drawOldResultsDetailScreen() {
    LGFX& lcd = display.getDisplay();
    lcd.fillScreen(BACKGROUND_COLOR);
    drawHeader("Rennen-Details", true);
    
    int y = HEADER_HEIGHT + 10;
    
    if (uiState.selectedOldRaceFile.length() == 0 || !dataLogger.exists(uiState.selectedOldRaceFile)) {
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_LARGE);
        lcd.setTextDatum(TC_DATUM);
        lcd.drawString("Datei nicht", SCREEN_WIDTH / 2, y + 50);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.drawString("gefunden", SCREEN_WIDTH / 2, y + 90);
        return;
    }
    
    // Read and parse CSV file
    String content = dataLogger.readFile(uiState.selectedOldRaceFile);
    if (content.length() == 0) {
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_LARGE);
        lcd.setTextDatum(TC_DATUM);
        lcd.drawString("Keine Daten", SCREEN_WIDTH / 2, y + 50);
        return;
    }
    
    // Parse CSV and build leaderboard
    // Format: Team ID,Team Name,Lap Number,Timestamp (ms),Duration (ms),Time of Day
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
        line.trim();
        
        if (firstLine) {
            firstLine = false;
            lineStart = lineEnd + 1;
            continue;  // Skip header
        }
        
        if (line.length() == 0) {
            lineStart = lineEnd + 1;
            continue;
        }
        
        // Parse CSV line: Team ID,Team Name,Lap Number,Timestamp,Duration,Time of Day
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
        // lapNumber - skip
        
        pos = commaPos + 1;
        commaPos = line.indexOf(',', pos);
        if (commaPos < 0) {
            lineStart = lineEnd + 1;
            continue;
        }
        // timestamp - skip
        
        pos = commaPos + 1;
        commaPos = line.indexOf(',', pos);
        if (commaPos < 0) {
            lineStart = lineEnd + 1;
            continue;
        }
        uint32_t duration = (uint32_t)line.substring(pos, commaPos).toInt();
        
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
    }
    
    // Convert to vector and sort by lap count
    std::vector<TeamStats> leaderboard;
    for (auto& pair : teamStats) {
        leaderboard.push_back(pair.second);
    }
    
    std::sort(leaderboard.begin(), leaderboard.end(), [](const TeamStats& a, const TeamStats& b) {
        if (a.lapCount != b.lapCount) {
            return a.lapCount > b.lapCount;
        }
        return a.totalDuration < b.totalDuration;
    });
    
    // Display filename
    String displayName = uiState.selectedOldRaceFile;
    if (displayName.startsWith("/races/")) {
        displayName = displayName.substring(7);
    }
    if (displayName.length() > 25) {
        displayName = displayName.substring(0, 22) + "...";
    }
    lcd.setTextColor(TEXT_COLOR);
    lcd.setTextSize(TEXT_SIZE_NORMAL);
    lcd.setCursor(10, y);
    lcd.print(displayName);
    
    y += 20;
    
    // Calculate max visible teams
    int availableHeight = SCREEN_HEIGHT - y - BUTTON_MARGIN;
    int itemH = LIST_ITEM_HEIGHT;
    int itemSpacing = BUTTON_MARGIN;
    int maxVisible = availableHeight / (itemH + itemSpacing);
    if (maxVisible < 2) maxVisible = 2;
    
    bool needsScroll = (int)leaderboard.size() > maxVisible;
    
    // Up arrow (if scrolling)
    if (needsScroll && uiState.resultsScrollOffset > 0) {
        lcd.fillTriangle(SCREEN_WIDTH - 25, y + 2, 
                        SCREEN_WIDTH - 15, y + 12, 
                        SCREEN_WIDTH - 35, y + 12, 
                        COLOR_SECONDARY);
    }
    
    // Display teams with scroll offset
    int displayCount = 0;
    int startIndex = uiState.resultsScrollOffset;
    int pos = startIndex + 1;
    
    for (size_t i = startIndex; i < leaderboard.size() && displayCount < maxVisible; i++) {
        const TeamStats& stats = leaderboard[i];
        
        // Position & Name
        lcd.setTextColor(TEXT_COLOR);
        lcd.setTextSize(TEXT_SIZE_NORMAL);
        lcd.setCursor(10, y);
        String teamDisplayName = stats.teamName;
        if (teamDisplayName.length() > 15) {
            teamDisplayName = teamDisplayName.substring(0, 12) + "...";
        }
        lcd.printf("%u. %s", pos, teamDisplayName.c_str());
        
        // Laps
        lcd.setCursor(240, y);
        lcd.printf("%uR", stats.lapCount);
        
        // Best lap if available
        if (stats.bestLapDuration < UINT32_MAX) {
            lcd.setTextSize(TEXT_SIZE_NORMAL);
            lcd.setCursor(20, y + 22);
            lcd.printf("Beste: %lu.%03lu s", 
                      stats.bestLapDuration / 1000, stats.bestLapDuration % 1000);
        }
        
        y += itemH + itemSpacing;
        pos++;
        displayCount++;
    }
    
    // Down arrow (if scrolling)
    if (needsScroll && startIndex + displayCount < (int)leaderboard.size()) {
        lcd.fillTriangle(SCREEN_WIDTH - 25, y - 2, 
                        SCREEN_WIDTH - 15, y - 12, 
                        SCREEN_WIDTH - 35, y - 12, 
                        COLOR_SECONDARY);
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
    lcd.setTextSize(1);  // Smaller label (intentional - space constraint)
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
    lcd.setTextSize(1);  // Smaller label (intentional - space constraint)
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
    // EXCEPTION: Screens with explicit back button handlers (TEAMS, RACE_RESULTS)
    if (uiState.currentScreen != SCREEN_HOME && 
        uiState.currentScreen != SCREEN_TEAMS && 
        uiState.currentScreen != SCREEN_RACE_RESULTS) {
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
        case SCREEN_TEAM_BEACON_ASSIGN:
            handleBeaconAssignTouch(x, y);
            break;
        case SCREEN_BEACON_LIST:
            handleBeaconListTouch(x, y);
            break;
        case SCREEN_RACE_SETUP:
            handleRaceSetupTouch(x, y);
            break;
        case SCREEN_RACE_COUNTDOWN:
            handleRaceCountdownTouch(x, y);
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
        case SCREEN_OLD_RESULTS_LIST:
            handleOldResultsListTouch(x, y);
            break;
        case SCREEN_OLD_RESULTS_DETAIL:
            handleOldResultsDetailTouch(x, y);
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
    
    auto teams = lapCounter.getAllTeams();
    int itemY = HEADER_HEIGHT + 10;
    
    // Calculate max visible
    int availableHeight = SCREEN_HEIGHT - 35 - BUTTON_MARGIN - itemY;
    int maxVisible = availableHeight / (LIST_ITEM_HEIGHT + BUTTON_MARGIN);
    if (maxVisible < 3) maxVisible = 3;
    
    bool needsScroll = (int)teams.size() > maxVisible;
    
    // Up arrow touch (ganz oben, rechts) - scroll one PAGE up
    if (needsScroll && uiState.scrollOffset > 0) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, itemY, 30, 15)) {
            // Scroll page up (by maxVisible items)
            uiState.scrollOffset -= maxVisible;
            if (uiState.scrollOffset < 0) uiState.scrollOffset = 0;
            uiState.needsRedraw = true;
            return;
        }
    }
    
    // Team items with scroll offset
    int displayCount = 0;
    int startIndex = uiState.scrollOffset;
    
    for (size_t i = startIndex; i < teams.size() && displayCount < maxVisible; i++) {
        auto* team = teams[i];
        
        if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 45, LIST_ITEM_HEIGHT)) {
            // Edit this team
            uiState.editingTeamId = team->teamId;
            uiState.editingTeamName = team->teamName;
            uiState.changeScreen(SCREEN_TEAM_EDIT);
            return;
        }
        
        itemY += LIST_ITEM_HEIGHT + BUTTON_MARGIN;
        displayCount++;
    }
    
    // Down arrow touch (ganz unten, rechts) - scroll one PAGE down
    if (needsScroll && startIndex + displayCount < (int)teams.size()) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, itemY - 15, 30, 15)) {
            // Scroll page down (by maxVisible items)
            uiState.scrollOffset += maxVisible;
            // Don't scroll past last item
            int maxOffset = teams.size() - maxVisible;
            if (uiState.scrollOffset > maxOffset) uiState.scrollOffset = maxOffset;
            uiState.needsRedraw = true;
            return;
        }
    }
    
    // Add button (klein, ganz unten links)
    int btnSize = 35;
    int btnY = SCREEN_HEIGHT - btnSize;  // Ganz unten
    if (isTouchInRect(x, y, 2, btnY, btnSize, btnSize)) {
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
    // Back button is handled by global handleTouch() at (5, 5, 40, 30)
    // No need for explicit handler here - global handler returns before reaching this function
    
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
        // BLE scanning will be started automatically by loop()
        return;
    }
    
    btnY += 70;
    
    // Remove beacon assignment button (only if beacon is assigned)
    TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
    bool hasBeacon = (team && team->beaconUUID.length() > 0);
    if (hasBeacon) {
        if (isTouchInRect(x, y, 10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT)) {
            if (team) {
                team->beaconUUID = "";
                persistence.saveTeams(lapCounter);
                showMessage("Entfernt", "Beacon-Zuordnung entfernt", COLOR_SECONDARY);
                uiState.needsRedraw = true;
            }
            return;
        }
        btnY += BUTTON_HEIGHT + BUTTON_MARGIN;
    }
    
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
    Serial.printf("[BeaconAssignTouch] x=%u, y=%u\n", x, y);
    
    // Back button
    if (isTouchInRect(x, y, 5, 5, 60, HEADER_HEIGHT)) {
        Serial.println("[BeaconAssign] Back button pressed");
        bleScanner.stopScan();
        uiState.changeScreen(uiState.previousScreen);
        return;
    }
    
    // Assign nearest beacon button (if visible and close enough)
    BeaconData* nearest = bleScanner.getNearestBeacon();
    Serial.printf("[BeaconAssign] Nearest beacon: %s\n", nearest ? "FOUND" : "NULL");
    
    if (nearest) {
        float dist = BLEScanner::rssiToDistance(nearest->rssi, nearest->txPower);
        
        Serial.printf("[BeaconAssign] MAC=%s, RSSI=%d, dist=%.2fm\n", 
                     nearest->macAddress.c_str(), nearest->rssi, dist);
        
        if (dist < 1.0) {
            // CRITICAL: Match exact Y position from drawTeamBeaconAssignScreen!
            // y = HEADER_HEIGHT + 12 + 35 (instructions) + 65 (boxH) + 12 (spacing)
            int btnY = HEADER_HEIGHT + 12 + 35 + 65 + 12;  // = 164
            int btnW = (SCREEN_WIDTH - 30) / 2;
            
            Serial.printf("[BeaconAssign] Buttons at Y=%d (h=%d), Touch at Y=%u\n", btnY, BUTTON_HEIGHT, y);
            Serial.printf("[BeaconAssign] Zuordnen: x=10-%d, y=%d-%d\n", 10+btnW, btnY, btnY+BUTTON_HEIGHT);
            Serial.printf("[BeaconAssign] Liste: x=%d-%d, y=%d-%d\n", 20+btnW, 20+btnW*2, btnY, btnY+BUTTON_HEIGHT);
            
            // "Zuordnen" Button (links)
            if (isTouchInRect(x, y, 10, btnY, btnW, BUTTON_HEIGHT)) {
                Serial.println("[BeaconAssign] ZUORDNEN BUTTON PRESSED!");

                TeamData* team = lapCounter.getTeam(uiState.editingTeamId);
                if (team) {
                    bleScanner.stopScan();  // CRITICAL: Stop scan BEFORE showMessage!
                    team->beaconUUID = nearest->macAddress;
                    persistence.saveTeams(lapCounter);
                    showMessage("Zuordnung", "Beacon zugeordnet", COLOR_SECONDARY);
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
    
    // Filter out already assigned beacons
    std::vector<BeaconData> unassignedBeacons;
    for (auto& beacon : beacons) {
        if (!isBeaconAssigned(beacon.macAddress)) {
            unassignedBeacons.push_back(beacon);
        }
    }
    
    if (unassignedBeacons.empty()) {
        showMessage("Info", "Alle Beacons sind bereits zugeordnet", COLOR_WARNING);
        return;
    }
    
    int y_start = HEADER_HEIGHT + 12;  // After header
    y_start += 25;  // After "Gefunden: X Beacon(s)" text
    
    // Calculate max visible (match drawBeaconListScreen)
    int availableHeight = SCREEN_HEIGHT - y_start - BUTTON_MARGIN;
    int itemH = 42;
    int itemSpacing = BUTTON_MARGIN;
    int maxVisible = availableHeight / (itemH + itemSpacing);
    if (maxVisible < 3) maxVisible = 3;
    
    bool needsScroll = (int)unassignedBeacons.size() > maxVisible;
    
    // Up arrow touch (scroll one page up)
    if (needsScroll && uiState.scrollOffset > 0) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, y_start, 30, 15)) {
            uiState.scrollOffset -= maxVisible;
            if (uiState.scrollOffset < 0) uiState.scrollOffset = 0;
            uiState.needsRedraw = true;
            return;
        }
    }
    
    // Beacon items with scroll offset (only unassigned)
    int displayCount = 0;
    int startIndex = uiState.scrollOffset;
    
    for (size_t i = startIndex; i < unassignedBeacons.size() && displayCount < maxVisible; i++) {
        auto& beacon = unassignedBeacons[i];
        
        int itemY = y_start + (displayCount * (itemH + itemSpacing));
        
        // Touch area for this beacon (full item height, but narrower due to scroll arrow)
        if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 45, itemH)) {
            Serial.printf("[Touch] Beacon #%zu clicked: MAC=%s UUID=%s (RSSI=%d) at y=%d\n", 
                         i, beacon.macAddress.c_str(), beacon.uuid.c_str(), beacon.rssi, itemY);
            
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
                    
                    bleScanner.stopScan();  // CRITICAL: Stop scan BEFORE showMessage!
                    showMessage("Zugeordnet", "Beacon erfolgreich zugeordnet", COLOR_SECONDARY);
                    uiState.changeScreen(SCREEN_TEAM_EDIT);
                    return;
                }
            } else {
                // Don't stop scan here - user might try another beacon
                showMessage("Hinweis", "Beacon zu weit! Naher halten (<1m)", COLOR_WARNING);
                // Screen will auto-refresh after delay
            }
            return;
        }
        
        displayCount++;
    }
    
    // Down arrow touch (scroll one page down)
    int lastItemY = y_start + (displayCount - 1) * (itemH + itemSpacing);
    if (needsScroll && startIndex + displayCount < (int)unassignedBeacons.size()) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, lastItemY - 15, 30, 15)) {
            uiState.scrollOffset += maxVisible;
            // Don't scroll past last item
            int maxOffset = unassignedBeacons.size() - maxVisible;
            if (uiState.scrollOffset > maxOffset) uiState.scrollOffset = maxOffset;
            uiState.needsRedraw = true;
            return;
        }
    }
}

void handleRaceSetupTouch(uint16_t x, uint16_t y) {
    // Back button handled by global handler
    
    int yPos = HEADER_HEIGHT + 8;
    
    // Race Name input - USE KEYBOARD!
    int inputH = 35;
    // MATCH drawRaceSetupScreen: label at y, input at y+12
    if (isTouchInRect(x, y, 10, yPos + 12, SCREEN_WIDTH - 20, inputH)) {
        String newName = inputText("Rennen-Name", uiState.raceName);
        if (newName.length() > 0) {
            uiState.raceName = newName;
        }
        return;
    }
    
    // MATCH drawRaceSetupScreen: yPos += inputH + 12, then +12 for duration label
    yPos += inputH + 12;  // After input box
    yPos += 12;  // After "DAUER (MIN)" label
    
    // Duration controls - MATCH drawRaceSetupScreen
    int btnSize = 28;
    int btnWidth = 45;
    
    // "-5" Button (left) - x=10, w=45
    if (isTouchInRect(x, y, 10, yPos, btnWidth, btnSize)) {
        if (uiState.raceDuration >= 5) {
            uiState.raceDuration -= 5;
        } else {
            uiState.raceDuration = 1;
        }
        Serial.printf("[RaceSetup] Duration: %u min (-5)\n", uiState.raceDuration);
        uiState.needsRedraw = true;
        return;
    }
    
    // Time display (center) - x=60, w=100 - CLICKABLE for manual input
    if (isTouchInRect(x, y, 60, yPos, 100, btnSize)) {
        // TODO: Manual time input (keyboard)
        showMessage("Info", "Manuelle Eingabe kommt noch", COLOR_WARNING);
        return;
    }
    
    // "+5" Button (right) - x=165, w=45
    if (isTouchInRect(x, y, 165, yPos, btnWidth, btnSize)) {
        if (uiState.raceDuration <= 175) {
            uiState.raceDuration += 5;
        } else {
            uiState.raceDuration = 180;
        }
        Serial.printf("[RaceSetup] Duration: %u min (+5)\n", uiState.raceDuration);
        uiState.needsRedraw = true;
        return;
    }
    
    // "+15" Button (far right) - x=215, w=45
    if (isTouchInRect(x, y, 215, yPos, 45, btnSize)) {
        if (uiState.raceDuration <= 165) {
            uiState.raceDuration += 15;
        } else {
            uiState.raceDuration = 180;
        }
        Serial.printf("[RaceSetup] Duration: %u min (+15)\n", uiState.raceDuration);
        uiState.needsRedraw = true;
        return;
    }
    
    yPos += btnSize + 10 + 10;  // btnSize + spacing + teams label
    
    // Team list with scroll support
    int teamCount = lapCounter.getTeamCount();
    if (teamCount > 0) {
        auto teams = lapCounter.getAllTeams();
        
        // Calculate max visible (match drawRaceSetupScreen)
        int startBtnY = SCREEN_HEIGHT - BUTTON_HEIGHT - 4;
        int teamListHeight = startBtnY - yPos - 8;
        int teamH = 26;
        int teamSpacing = 4;
        int maxVisible = teamListHeight / (teamH + teamSpacing);
        if (maxVisible < 1) maxVisible = 1;
        
        bool needsScroll = (int)teams.size() > maxVisible;
        
        // Up arrow touch (scroll one page up)
        if (needsScroll && uiState.raceSetupScrollOffset > 0) {
            if (isTouchInRect(x, y, SCREEN_WIDTH - 30, yPos, 20, 15)) {
                uiState.raceSetupScrollOffset -= maxVisible;
                if (uiState.raceSetupScrollOffset < 0) uiState.raceSetupScrollOffset = 0;
                uiState.needsRedraw = true;
                return;
            }
        }
        
        // Team items with scroll offset
        int displayCount = 0;
        int startIndex = uiState.raceSetupScrollOffset;
        
        for (size_t i = startIndex; i < teams.size() && displayCount < maxVisible; i++) {
            TeamData* team = teams[i];
            if (!team) continue;
            
            int itemY = yPos + (displayCount * (teamH + teamSpacing));
            
            if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 35, teamH)) {
                // Toggle selection for this team ID (index = teamId - 1)
                uiState.selectedTeams[team->teamId - 1] = !uiState.selectedTeams[team->teamId - 1];
                Serial.printf("[RaceSetup] Team %u (%s) %s\n", 
                             team->teamId, team->teamName.c_str(), 
                             uiState.selectedTeams[team->teamId - 1] ? "selected" : "deselected");
                uiState.needsRedraw = true;
                return;
            }
            
            displayCount++;
        }
        
        // Down arrow touch (scroll one page down)
        int lastItemY = yPos + (displayCount - 1) * (teamH + teamSpacing);
        if (needsScroll && startIndex + displayCount < (int)teams.size()) {
            if (isTouchInRect(x, y, SCREEN_WIDTH - 30, lastItemY - 15, 20, 15)) {
                uiState.raceSetupScrollOffset += maxVisible;
                int maxOffset = teams.size() - maxVisible;
                if (uiState.raceSetupScrollOffset > maxOffset) uiState.raceSetupScrollOffset = maxOffset;
                uiState.needsRedraw = true;
                return;
            }
        }
    }
    
    // Start button - FIXED at bottom
    int btnY = SCREEN_HEIGHT - BUTTON_HEIGHT - 4;
    if (isTouchInRect(x, y, 10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT)) {
        if (lapCounter.getTeamCount() == 0) {
            showMessage("Fehler", "Keine Teams!", COLOR_DANGER);
            return;
        }
        
        // Prepare race (but don't start yet - go to countdown first)
        currentRaceName = uiState.raceName.length() > 0 ? uiState.raceName : "Rennen";
        raceDuration = uiState.raceDuration * 60 * 1000;  // Convert to ms
        
        // Save race config
        persistence.saveConfig(currentRaceName, uiState.raceDuration);
        
        // Reset all teams
        lapCounter.reset();
        
        // Reset beacon presence
        beaconPresence.clear();
        
        // CRITICAL: Go to countdown screen FIRST (race will start after countdown)
        extern uint32_t countdownStartTime;
        countdownStartTime = millis();
        uiState.changeScreen(SCREEN_RACE_COUNTDOWN);
        
        // Start logging/scanning during countdown (non-blocking)
        if (dataLogger.isReady()) {
            dataLogger.startNewRace(currentRaceName);
        }
        
        Serial.println("\n=== RACE COUNTDOWN STARTED ===");
        Serial.printf("Name: %s\n", currentRaceName.c_str());
        Serial.printf("Duration: %lu minutes\n", uiState.raceDuration);
        Serial.printf("Teams: %u\n", lapCounter.getTeamCount());
    }
}

void handleRaceCountdownTouch(uint16_t x, uint16_t y) {
    // Skip countdown on touch (optional - allow skipping)
    extern uint32_t countdownStartTime;
    if (millis() - countdownStartTime > 1000) {  // Only allow skip after 1 second
        // Skip to race start
        countdownStartTime = millis() - 10000;  // Force countdown to finish
    }
}

void handleRaceRunningTouch(uint16_t x, uint16_t y) {
    int btnSize = 40;
    int btnY = SCREEN_HEIGHT - btnSize - 2;
    
    // Pause button (links unten)
    if (isTouchInRect(x, y, 2, btnY, btnSize, btnSize)) {
        raceRunning = false;
        uiState.changeScreen(SCREEN_RACE_PAUSED);
        Serial.println("\n=== RACE PAUSED ===");
        return;
    }
    
    // Stop button (rechts unten)
    if (isTouchInRect(x, y, SCREEN_WIDTH - btnSize - 2, btnY, btnSize, btnSize)) {
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
    
    auto leaderboard = lapCounter.getLeaderboard(true);
    if (leaderboard.empty()) return;
    
    int y_start = HEADER_HEIGHT + 10 + 18;  // After header + "Aktuelles Rennen:" label
    
    // Calculate max visible (match drawRaceResultsScreen)
    int availableHeight = SCREEN_HEIGHT - y_start - BUTTON_MARGIN;
    int itemH = LIST_ITEM_HEIGHT;
    int itemSpacing = BUTTON_MARGIN;
    int maxVisible = availableHeight / (itemH + itemSpacing);
    if (maxVisible < 2) maxVisible = 2;
    
    bool needsScroll = (int)leaderboard.size() > maxVisible;
    
    // Up arrow touch (scroll one page up)
    if (needsScroll && uiState.resultsScrollOffset > 0) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, y_start, 30, 15)) {
            uiState.resultsScrollOffset -= maxVisible;
            if (uiState.resultsScrollOffset < 0) uiState.resultsScrollOffset = 0;
            uiState.needsRedraw = true;
            return;
        }
    }
    
    // Down arrow touch (scroll one page down)
    int lastItemY = y_start + (maxVisible - 1) * (itemH + itemSpacing);
    if (needsScroll && uiState.resultsScrollOffset + maxVisible < (int)leaderboard.size()) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, lastItemY - 15, 30, 15)) {
            uiState.resultsScrollOffset += maxVisible;
            int maxOffset = leaderboard.size() - maxVisible;
            if (uiState.resultsScrollOffset > maxOffset) uiState.resultsScrollOffset = maxOffset;
            uiState.needsRedraw = true;
            return;
        }
    }
    
    // "Alte Ergebnisse" button (if SD card is ready)
    if (dataLogger.isReady()) {
        int btnY = SCREEN_HEIGHT - BUTTON_HEIGHT - 4;
        if (isTouchInRect(x, y, 10, btnY, SCREEN_WIDTH - 20, BUTTON_HEIGHT)) {
            uiState.changeScreen(SCREEN_OLD_RESULTS_LIST);
            return;
        }
    }
}

void handleOldResultsListTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, 5, 5, 60, HEADER_HEIGHT)) {
        uiState.changeScreen(SCREEN_RACE_RESULTS);
        return;
    }
    
    // Get list of race files
    String raceFiles = dataLogger.getRaceFileList(20);
    if (raceFiles.length() == 0) return;
    
    // Parse race files into array
    std::vector<String> files;
    int startPos = 0;
    int newlinePos = raceFiles.indexOf('\n', startPos);
    
    while (newlinePos >= 0) {
        String filename = raceFiles.substring(startPos, newlinePos);
        if (filename.length() > 0) {
            files.push_back("/races/" + filename);
        }
        startPos = newlinePos + 1;
        newlinePos = raceFiles.indexOf('\n', startPos);
    }
    
    if (startPos < raceFiles.length()) {
        String filename = raceFiles.substring(startPos);
        if (filename.length() > 0) {
            files.push_back("/races/" + filename);
        }
    }
    
    int y_start = HEADER_HEIGHT + 10;
    int itemH = 30;
    int itemSpacing = 4;
    int availableHeight = SCREEN_HEIGHT - y_start - BUTTON_MARGIN;
    int maxVisible = availableHeight / (itemH + itemSpacing);
    if (maxVisible < 2) maxVisible = 2;
    
    bool needsScroll = (int)files.size() > maxVisible;
    
    // Up arrow touch
    if (needsScroll && uiState.oldResultsListScrollOffset > 0) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, y_start, 30, 15)) {
            uiState.oldResultsListScrollOffset -= maxVisible;
            if (uiState.oldResultsListScrollOffset < 0) uiState.oldResultsListScrollOffset = 0;
            uiState.needsRedraw = true;
            return;
        }
    }
    
    // File items
    int displayCount = 0;
    int startIndex = uiState.oldResultsListScrollOffset;
    
    for (size_t i = startIndex; i < files.size() && displayCount < maxVisible; i++) {
        int itemY = y_start + (displayCount * (itemH + itemSpacing));
        
        if (isTouchInRect(x, y, 10, itemY, SCREEN_WIDTH - 35, itemH)) {
            // Selected old race file - parse and show results
            String filename = files[i];
            uiState.selectedOldRaceFile = filename;
            uiState.changeScreen(SCREEN_OLD_RESULTS_DETAIL);
            return;
        }
        
        displayCount++;
    }
    
    // Down arrow touch
    if (needsScroll && startIndex + displayCount < (int)files.size()) {
        int lastItemY = y_start + (displayCount - 1) * (itemH + itemSpacing);
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, lastItemY - 15, 30, 15)) {
            uiState.oldResultsListScrollOffset += maxVisible;
            int maxOffset = files.size() - maxVisible;
            if (uiState.oldResultsListScrollOffset > maxOffset) uiState.oldResultsListScrollOffset = maxOffset;
            uiState.needsRedraw = true;
            return;
        }
    }
}

void handleOldResultsDetailTouch(uint16_t x, uint16_t y) {
    // Back button
    if (isTouchInRect(x, y, 5, 5, 60, HEADER_HEIGHT)) {
        uiState.changeScreen(SCREEN_OLD_RESULTS_LIST);
        return;
    }
    
    // Scroll support (same as results screen)
    if (uiState.selectedOldRaceFile.length() == 0 || !dataLogger.exists(uiState.selectedOldRaceFile)) {
        return;
    }
    
    String content = dataLogger.readFile(uiState.selectedOldRaceFile);
    if (content.length() == 0) return;
    
    // Quick parse to count teams (for scroll calculation)
    int teamCount = 0;
    int lineStart = 0;
    bool firstLine = true;
    while (lineStart < content.length()) {
        int lineEnd = content.indexOf('\n', lineStart);
        if (lineEnd < 0) lineEnd = content.length();
        String line = content.substring(lineStart, lineEnd);
        line.trim();
        
        if (firstLine) {
            firstLine = false;
            lineStart = lineEnd + 1;
            continue;
        }
        
        if (line.length() > 0 && line.indexOf(',') >= 0) {
            teamCount++;
        }
        lineStart = lineEnd + 1;
    }
    
    int y_start = HEADER_HEIGHT + 10 + 20;  // After header + filename
    int availableHeight = SCREEN_HEIGHT - y_start - BUTTON_MARGIN;
    int itemH = LIST_ITEM_HEIGHT;
    int itemSpacing = BUTTON_MARGIN;
    int maxVisible = availableHeight / (itemH + itemSpacing);
    if (maxVisible < 2) maxVisible = 2;
    
    bool needsScroll = teamCount > maxVisible;
    
    // Up arrow touch
    if (needsScroll && uiState.resultsScrollOffset > 0) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, y_start, 30, 15)) {
            uiState.resultsScrollOffset -= maxVisible;
            if (uiState.resultsScrollOffset < 0) uiState.resultsScrollOffset = 0;
            uiState.needsRedraw = true;
            return;
        }
    }
    
    // Down arrow touch
    int lastItemY = y_start + (maxVisible - 1) * (itemH + itemSpacing);
    if (needsScroll && uiState.resultsScrollOffset + maxVisible < teamCount) {
        if (isTouchInRect(x, y, SCREEN_WIDTH - 40, lastItemY - 15, 30, 15)) {
            uiState.resultsScrollOffset += maxVisible;
            int maxOffset = teamCount - maxVisible;
            if (uiState.resultsScrollOffset > maxOffset) uiState.resultsScrollOffset = maxOffset;
            uiState.needsRedraw = true;
            return;
        }
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
            // Confirmation dialog with OK/Cancel buttons
            if (showConfirmationDialog("WARNUNG!", "Alle Daten werden geloscht!", COLOR_DANGER)) {
                // User confirmed - format SD card
                if (dataLogger.formatSD()) {
                    showMessage("Erfolg", "SD-Karte formatiert", COLOR_SECONDARY);
                    // Reinitialize SD after format
                    dataLogger.begin(SD_CS_PIN);
                } else {
                    showMessage("Fehler", "Formatierung fehlgeschlagen", COLOR_DANGER);
                }
            }
            // User cancelled - do nothing
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

