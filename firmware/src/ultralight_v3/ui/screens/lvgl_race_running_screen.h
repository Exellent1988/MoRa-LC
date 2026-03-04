#ifndef LVGL_RACE_RUNNING_SCREEN_H
#define LVGL_RACE_RUNNING_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/beacon_service.h"
#include "../../services/lap_counter_service.h"
#include <lvgl.h>

class LVGLNavigation;
class LVGLRaceResultsScreen;
class DataLoggerService;

class LVGLRaceRunningScreen : public LVGLBaseScreen {
public:
    LVGLRaceRunningScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService, LapCounterService* lapCounter);
    virtual ~LVGLRaceRunningScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override;
    
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    void setRaceResultsScreen(LVGLRaceResultsScreen* screen) { _raceResultsScreen = screen; }
    void setDataLogger(DataLoggerService* logger) { _dataLogger = logger; }
    
    void startRace(uint32_t durationMinutes, const char* raceName = nullptr);
    void pauseRace();
    void stopRace();
    
private:
    LVGLNavigation* _navigation;
    BeaconService* _beaconService;
    LapCounterService* _lapCounter;
    LVGLRaceResultsScreen* _raceResultsScreen;
    DataLoggerService* _dataLogger;
    
    // Two-view system: time view (default) and board view
    bool _showBoard;
    
    // Time view elements
    lv_obj_t* _timeViewContainer;
    lv_obj_t* _bigTimeLabel;
    lv_obj_t* _bigRemainingLabel;
    
    // Board view elements
    lv_obj_t* _boardViewContainer;
    lv_obj_t* _smallTimeLabel;
    lv_obj_t* _leaderboardList;
    
    // Shared elements
    lv_obj_t* _timeLabel;       // alias for active time label
    lv_obj_t* _remainingLabel;  // alias for active remaining label
    lv_obj_t* _pauseButton;
    lv_obj_t* _stopButton;
    lv_obj_t* _viewButton;
    
    // Race state
    uint32_t _raceStartTime;
    uint32_t _raceDuration;
    bool _isRunning;
    bool _isPaused;
    uint32_t _pausedTime;
    uint32_t _lastUpdate;
    
    uint32_t _lastDisplayedTime;
    uint32_t _lastDisplayedRemaining;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    static void pauseBtnEventHandler(lv_event_t* e);
    static void stopBtnEventHandler(lv_event_t* e);
    static void viewBtnEventHandler(lv_event_t* e);
    
    void switchView();
    void updateLeaderboard();
    uint32_t getElapsedTime() const;
    uint32_t getRemainingTime() const;
    String formatTime(uint32_t milliseconds) const;
};

#endif // LVGL_RACE_RUNNING_SCREEN_H
