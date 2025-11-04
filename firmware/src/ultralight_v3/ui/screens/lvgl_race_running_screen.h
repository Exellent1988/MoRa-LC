#ifndef LVGL_RACE_RUNNING_SCREEN_H
#define LVGL_RACE_RUNNING_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/beacon_service.h"
#include "../../services/lap_counter_service.h"
#include <lvgl.h>

// Forward declaration
class LVGLNavigation;

/**
 * Race Running Screen (LVGL8)
 * Display race progress, lap counts, and leaderboard
 * CRITICAL: Optimized for performance - only updates changed values
 */
class LVGLRaceRunningScreen : public LVGLBaseScreen {
public:
    LVGLRaceRunningScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService, LapCounterService* lapCounter);
    virtual ~LVGLRaceRunningScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override;  // Called frequently for dynamic updates
    
    // Set navigation for back button
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
    // Race control
    void startRace(uint32_t durationMinutes);
    void pauseRace();
    void stopRace();
    
private:
    LVGLNavigation* _navigation;
    BeaconService* _beaconService;
    LapCounterService* _lapCounter;
    
    // UI elements
    lv_obj_t* _timeLabel;
    lv_obj_t* _remainingLabel;
    lv_obj_t* _leaderboardList;
    lv_obj_t* _pauseButton;
    lv_obj_t* _stopButton;
    
    // Race state
    uint32_t _raceStartTime;
    uint32_t _raceDuration;  // in milliseconds
    bool _isRunning;
    bool _isPaused;
    uint32_t _pausedTime;
    uint32_t _lastUpdate;
    
    // Performance optimization: Only update if values changed
    uint32_t _lastDisplayedTime;
    uint32_t _lastDisplayedRemaining;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    static void pauseBtnEventHandler(lv_event_t* e);
    static void stopBtnEventHandler(lv_event_t* e);
    
    // Helper methods
    void updateTimeDisplay();
    void updateLeaderboard();
    uint32_t getElapsedTime() const;
    uint32_t getRemainingTime() const;
    String formatTime(uint32_t milliseconds) const;
};

#endif // LVGL_RACE_RUNNING_SCREEN_H

