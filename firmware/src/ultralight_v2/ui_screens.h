#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include <Arduino.h>
#include "display_manager.h"
#include "ui_state.h"
#include "config.h"
#include "../../lib/BLEScanner/BLEScanner.h"
#include "../../lib/LapCounter/LapCounter.h"

// Forward declarations
extern DisplayManager display;
extern BLEScanner bleScanner;
extern LapCounter lapCounter;
class DataLogger;  // Forward declaration
extern DataLogger dataLogger;
extern int8_t lapRssiNear;
extern int8_t lapRssiFar;

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
void handleSettingsTouch(uint16_t x, uint16_t y);

// Helper Functions (from ui_helper.h) - declarations here for convenience

#endif // UI_SCREENS_H

