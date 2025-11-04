#ifndef LVGL_HOME_SCREEN_H
#define LVGL_HOME_SCREEN_H

#include "lvgl_base_screen.h"
#include <lvgl.h>

// Forward declarations
class LVGLNavigation;
class LVGLTeamsScreen;
class LVGLRaceSetupScreen;
class LVGLRaceResultsScreen;
class LVGLSettingsScreen;

/**
 * Home Screen (LVGL8)
 * Main navigation screen with large icon buttons
 */
class LVGLHomeScreen : public LVGLBaseScreen {
public:
    LVGLHomeScreen(LVGLDisplay* lvglDisplay);
    virtual ~LVGLHomeScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    // Set navigation for screen transitions
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
    // Set screen references (for navigation)
    void setTeamsScreen(LVGLTeamsScreen* screen) { _teamsScreen = screen; }
    void setRaceSetupScreen(LVGLRaceSetupScreen* screen) { _raceSetupScreen = screen; }
    void setRaceResultsScreen(LVGLRaceResultsScreen* screen) { _raceResultsScreen = screen; }
    void setSettingsScreen(LVGLSettingsScreen* screen) { _settingsScreen = screen; }
    
private:
    LVGLNavigation* _navigation;
    LVGLTeamsScreen* _teamsScreen;
    LVGLRaceSetupScreen* _raceSetupScreen;
    LVGLRaceResultsScreen* _raceResultsScreen;
    LVGLSettingsScreen* _settingsScreen;
    lv_obj_t* _btnRace;
    lv_obj_t* _btnTeams;
    lv_obj_t* _btnResults;
    lv_obj_t* _btnSettings;
    
    // Event handlers
    static void btnRaceEventHandler(lv_event_t* e);
    static void btnTeamsEventHandler(lv_event_t* e);
    static void btnResultsEventHandler(lv_event_t* e);
    static void btnSettingsEventHandler(lv_event_t* e);
};

#endif // LVGL_HOME_SCREEN_H

