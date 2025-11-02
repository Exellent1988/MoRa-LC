#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "BLEScanner.h"
#include "LapCounter.h"

/**
 * UI Screens für UltraLight
 * 
 * Verwaltet alle Screen-Zeichnungen und Touch-Handler
 */

// Forward declarations
extern TFT_eSPI tft;
extern BLEScanner bleScanner;
extern LapCounter lapCounter;

// Screen States
enum Screen {
    SCREEN_HOME,
    SCREEN_TEAMS,
    SCREEN_TEAM_EDIT,
    SCREEN_TEAM_BEACON_ASSIGN,
    SCREEN_BEACON_LIST,
    SCREEN_RACE_SETUP,
    SCREEN_RACE_RUNNING,
    SCREEN_RACE_PAUSED,
    SCREEN_RACE_RESULTS,
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
    
    // Touch
    uint16_t touchX;
    uint16_t touchY;
    bool touched;
    uint32_t lastTouchTime;
    
    UIState() : currentScreen(SCREEN_HOME), previousScreen(SCREEN_HOME),
                needsRedraw(true), editingTeamId(0), editingTeamName(""),
                scrollOffset(0), raceName("Rennen"), raceDuration(60),
                touchX(0), touchY(0), touched(false), lastTouchTime(0) {
        memset(selectedTeams, false, sizeof(selectedTeams));
    }
};

extern UIState uiState;

// Screen Drawing Functions
void drawHomeScreen();
void drawTeamsScreen();
void drawTeamEditScreen();
void drawTeamBeaconAssignScreen();
void drawBeaconListScreen();
void drawRaceSetupScreen();
void drawRaceRunningScreen();
void drawRacePausedScreen();
void drawRaceResultsScreen();
void drawSettingsScreen();

// Touch Handler
void handleTouch(uint16_t x, uint16_t y);
void handleHomeTouch(uint16_t x, uint16_t y);
void handleTeamsTouch(uint16_t x, uint16_t y);
void handleTeamEditTouch(uint16_t x, uint16_t y);
void handleBeaconAssignTouch(uint16_t x, uint16_t y);
void handleBeaconListTouch(uint16_t x, uint16_t y);
void handleRaceSetupTouch(uint16_t x, uint16_t y);
void handleRaceRunningTouch(uint16_t x, uint16_t y);
void handleRacePausedTouch(uint16_t x, uint16_t y);
void handleRaceResultsTouch(uint16_t x, uint16_t y);

// Helper Functions
void drawHeader(const String& title, bool showBack = false);
void drawButton(int x, int y, int w, int h, const String& text, uint16_t color = COLOR_BUTTON);
bool isTouchInRect(uint16_t tx, uint16_t ty, int x, int y, int w, int h);
void showMessage(const String& title, const String& message, uint16_t color = TFT_WHITE);
String inputText(const String& prompt, const String& defaultValue = "");
uint32_t inputNumber(const String& prompt, uint32_t defaultValue, uint32_t min, uint32_t max);

#endif // UI_SCREENS_H


