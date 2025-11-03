#ifndef UI_STATE_H
#define UI_STATE_H

#include <Arduino.h>
#include "config.h"

/**
 * UI State Management für UltraLight v2
 */

// Screen States
enum Screen {
    SCREEN_HOME,
    SCREEN_TEAMS,
    SCREEN_TEAM_EDIT,
    SCREEN_TEAM_BEACON_ASSIGN,
    SCREEN_BEACON_LIST,
    SCREEN_RACE_SETUP,
    SCREEN_RACE_COUNTDOWN,
    SCREEN_RACE_RUNNING,
    SCREEN_RACE_PAUSED,
    SCREEN_RACE_RESULTS,
    SCREEN_OLD_RESULTS_LIST,
    SCREEN_OLD_RESULTS_DETAIL,
    SCREEN_SETTINGS
};

// UI State
struct UIState {
    Screen currentScreen;
    Screen previousScreen;
    bool needsRedraw;
    
    // Team Edit
    uint8_t editingTeamId;
    String editingTeamName;
    int scrollOffset;
    
    // Race Setup
    String raceName;
    uint32_t raceDuration;  // minutes
    bool selectedTeams[MAX_TEAMS];
    int raceSetupScrollOffset;  // Separate scroll offset for race setup team list
    
    // Race Running
    int raceRunningScrollOffset;  // Scroll offset for race running leaderboard
    uint32_t lastAutoScrollTime;  // For auto-scroll during race
    
    // Results
    uint8_t resultsPage;  // 0 = current, 1-3 = historical
    int resultsScrollOffset;  // Scroll offset for results list
    String selectedOldRaceFile;  // Selected old race file path
    int oldResultsListScrollOffset;  // Scroll offset for old results list
    
    // Beacon List Cache (für Change-Detection)
    uint32_t lastBeaconHash;
    uint32_t lastBeaconCount;
    
    // Touch
    uint16_t touchX;
    uint16_t touchY;
    bool touched;
    uint32_t lastTouchTime;
    
    UIState() : currentScreen(SCREEN_HOME), previousScreen(SCREEN_HOME),
                needsRedraw(true), editingTeamId(0), editingTeamName(""),
                scrollOffset(0), raceName("Rennen"), raceDuration(60),
                raceSetupScrollOffset(0), raceRunningScrollOffset(0),
                lastAutoScrollTime(0), lastBeaconHash(0), lastBeaconCount(0),
                touchX(0), touchY(0), touched(false), lastTouchTime(0),
                resultsPage(0), resultsScrollOffset(0), oldResultsListScrollOffset(0) {
        memset(selectedTeams, false, sizeof(selectedTeams));
    }
    
    void changeScreen(Screen newScreen) {
        previousScreen = currentScreen;
        currentScreen = newScreen;
        needsRedraw = true;
        scrollOffset = 0;
        raceSetupScrollOffset = 0;
        raceRunningScrollOffset = 0;
        resultsScrollOffset = 0;
        oldResultsListScrollOffset = 0;
        selectedOldRaceFile = "";
        lastBeaconHash = 0; // Reset beacon cache
        lastBeaconCount = 0;
        lastAutoScrollTime = 0;
    }
};

extern UIState uiState;

#endif // UI_STATE_H

