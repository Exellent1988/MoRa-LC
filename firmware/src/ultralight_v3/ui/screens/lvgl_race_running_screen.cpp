#include "lvgl_race_running_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLRaceRunningScreen::LVGLRaceRunningScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _beaconService(beaconService)
    , _lapCounter(lapCounter)
    , _raceResultsScreen(nullptr)
    , _timeLabel(nullptr)
    , _remainingLabel(nullptr)
    , _leaderboardList(nullptr)
    , _pauseButton(nullptr)
    , _stopButton(nullptr)
    , _raceStartTime(0)
    , _raceDuration(0)
    , _isRunning(false)
    , _isPaused(false)
    , _pausedTime(0)
    , _lastUpdate(0)
    , _lastDisplayedTime(0)
    , _lastDisplayedRemaining(0) {
}

LVGLRaceRunningScreen::~LVGLRaceRunningScreen() {
}

void LVGLRaceRunningScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLRaceRunning] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLRaceRunning] Creating race running screen...");
    Serial.printf("[LVGLRaceRunning] Display: %p, BeaconService: %p, LapCounter: %p\n",
                  _lvglDisplay, _beaconService, _lapCounter);
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    if (!_screen) {
        Serial.println("[LVGLRaceRunning] ERROR: Could not get screen");
        return;
    }
    
    // Clear screen and reset UI elements
    lv_obj_clean(_screen);
    _timeLabel = nullptr;
    _remainingLabel = nullptr;
    _leaderboardList = nullptr;
    _pauseButton = nullptr;
    _stopButton = nullptr;
    
    // Set background color
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    // Create header (no back button during race, ASCII only)
    createHeader("Race Running", false);
    
    // Time display (large, centered)
    int timeY = HEADER_HEIGHT + Spacing::MD;
    _timeLabel = createLabel("00:00:00", 0, timeY, SCREEN_WIDTH, 50, rgb565ToLVGL(Colors::TEXT));
    if (_timeLabel) {
        lv_obj_set_style_text_align(_timeLabel, LV_TEXT_ALIGN_CENTER, 0);
    }
    
    // Remaining time (ASCII only)
    int remainingY = timeY + 60;
    _remainingLabel = createLabel("Remaining: 00:00", 0, remainingY, SCREEN_WIDTH, 30, rgb565ToLVGL(Colors::TEXT_SECONDARY));
    if (_remainingLabel) {
        lv_obj_set_style_text_align(_remainingLabel, LV_TEXT_ALIGN_CENTER, 0);
    }
    
    // Leaderboard list
    int listY = remainingY + 40;
    int listH = SCREEN_HEIGHT - listY - BUTTON_HEIGHT - Spacing::MD;
    _leaderboardList = createList(Spacing::MD, listY, SCREEN_WIDTH - 2 * Spacing::MD, listH);
    
    // Control buttons (bottom)
    int btnY = SCREEN_HEIGHT - BUTTON_HEIGHT - Spacing::MD;
    int btnW = (SCREEN_WIDTH - 3 * Spacing::MD) / 2;
    
    _pauseButton = createButton("Pause", Spacing::MD, btnY, btnW, BUTTON_HEIGHT, pauseBtnEventHandler, this);
    _stopButton = createButton("Stop", Spacing::MD * 2 + btnW, btnY, btnW, BUTTON_HEIGHT, stopBtnEventHandler, this);
    if (_stopButton) {
        lv_obj_set_style_bg_color(_stopButton, rgb565ToLVGL(Colors::DANGER), 0);
    }
    
    _lastUpdate = millis();
    _lastDisplayedTime = 0;
    _lastDisplayedRemaining = 0;
    
    // Update leaderboard if race is already running
    if (_isRunning && _leaderboardList) {
        updateLeaderboard();
    }
    
    Serial.println("[LVGLRaceRunning] Race running screen created");
}

void LVGLRaceRunningScreen::onExit() {
    stopRace();
}

void LVGLRaceRunningScreen::update() {
    if (!_isRunning || !_timeLabel || !_remainingLabel) return;
    
    // Performance: Only update display every 100ms (10fps for time display is enough)
    uint32_t now = millis();
    if (now - _lastUpdate < 100) return;
    _lastUpdate = now;
    
    uint32_t elapsed = getElapsedTime();
    uint32_t remaining = getRemainingTime();
    
    // Only update if values changed (performance optimization)
    if (elapsed != _lastDisplayedTime) {
        String timeStr = formatTime(elapsed);
        lv_label_set_text(_timeLabel, timeStr.c_str());
        _lastDisplayedTime = elapsed;
    }
    
    if (remaining != _lastDisplayedRemaining) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Remaining: %s", formatTime(remaining).c_str());
        lv_label_set_text(_remainingLabel, buf);
        _lastDisplayedRemaining = remaining;
    }
    
    // Update leaderboard less frequently (every 500ms)
    static uint32_t lastLeaderboardUpdate = 0;
    if (now - lastLeaderboardUpdate > 500) {
        updateLeaderboard();
        lastLeaderboardUpdate = now;
    }
}

void LVGLRaceRunningScreen::startRace(uint32_t durationMinutes) {
    Serial.printf("[LVGLRaceRunning] startRace called - Duration: %lu minutes\n", durationMinutes);
    Serial.printf("[LVGLRaceRunning] BeaconService: %p, LapCounter: %p\n", _beaconService, _lapCounter);
    
    // Set race state FIRST (before enabling services)
    _raceStartTime = millis();
    _raceDuration = durationMinutes * 60 * 1000;  // Convert to milliseconds
    _isRunning = true;
    _isPaused = false;
    _pausedTime = 0;
    _lastUpdate = millis();
    _lastDisplayedTime = 0;
    _lastDisplayedRemaining = 0;
    
    // Enable race mode in beacon service
    if (_beaconService) {
        Serial.println("[LVGLRaceRunning] Enabling race mode in beacon service...");
        _beaconService->setRaceMode(true);
        _beaconService->startScanning(0);
        Serial.println("[LVGLRaceRunning] Beacon service race mode enabled");
    } else {
        Serial.println("[LVGLRaceRunning] WARNING: BeaconService is null!");
    }
    
    // Start race in lap counter
    if (_lapCounter) {
        Serial.println("[LVGLRaceRunning] Starting race in lap counter...");
        _lapCounter->startRace();
        Serial.println("[LVGLRaceRunning] Lap counter race started");
    } else {
        Serial.println("[LVGLRaceRunning] WARNING: LapCounter is null!");
    }
    
    Serial.printf("[LVGLRaceRunning] Race started successfully - Duration: %lu minutes\n", durationMinutes);
}

void LVGLRaceRunningScreen::pauseRace() {
    if (!_isRunning) return;
    
    if (_isPaused) {
        // Resume
        _raceStartTime += (millis() - _pausedTime);
        _isPaused = false;
        Serial.println("[LVGLRaceRunning] Race resumed");
    } else {
        // Pause
        _pausedTime = millis();
        _isPaused = true;
        Serial.println("[LVGLRaceRunning] Race paused");
    }
}

void LVGLRaceRunningScreen::stopRace() {
    _isRunning = false;
    _isPaused = false;
    
    // Disable race mode in beacon service
    if (_beaconService) {
        _beaconService->setRaceMode(false);
        _beaconService->stopScanning();
    }
    
    // Stop race in lap counter
    if (_lapCounter) {
        _lapCounter->stopRace();
    }
    
    Serial.println("[LVGLRaceRunning] Race stopped");
    
    // Navigate to results screen
    if (_navigation && _raceResultsScreen) {
        Serial.println("[LVGLRaceRunning] Navigating to results screen");
        _navigation->setScreen((LVGLBaseScreen*)_raceResultsScreen);
    }
}

void LVGLRaceRunningScreen::updateTimeDisplay() {
    if (!_timeLabel || !_remainingLabel) return;
    
    uint32_t elapsed = getElapsedTime();
    uint32_t remaining = getRemainingTime();
    
    String timeStr = formatTime(elapsed);
    lv_label_set_text(_timeLabel, timeStr.c_str());
    
    char buf[64];
    snprintf(buf, sizeof(buf), "Verbleibend: %s", formatTime(remaining).c_str());
    lv_label_set_text(_remainingLabel, buf);
}

void LVGLRaceRunningScreen::updateLeaderboard() {
    if (!_leaderboardList) return;
    
    // Clear existing list items
    lv_obj_clean(_leaderboardList);
    
    if (!_lapCounter) {
        lv_obj_t* item = lv_list_add_btn(_leaderboardList, LV_SYMBOL_FILE, "LapCounter not available");
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    std::vector<TeamData*> leaderboard = _lapCounter->getLeaderboard();
    
    if (leaderboard.empty()) {
        lv_obj_t* item = lv_list_add_btn(_leaderboardList, LV_SYMBOL_FILE, "No teams");
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    // Add teams to leaderboard (max 4 visible)
    int count = 0;
    for (TeamData* team : leaderboard) {
        if (count >= 4) break; // Max 4 teams visible
        
        char info[128];
        snprintf(info, sizeof(info), "%u. %s\nLaps: %u", 
                count + 1, team->teamName.c_str(), team->lapCount);
        
        lv_obj_t* item = lv_list_add_btn(_leaderboardList, LV_SYMBOL_FILE, info);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        
        count++;
    }
}

uint32_t LVGLRaceRunningScreen::getElapsedTime() const {
    if (!_isRunning) return 0;
    
    if (_isPaused) {
        return _pausedTime - _raceStartTime;
    } else {
        return millis() - _raceStartTime;
    }
}

uint32_t LVGLRaceRunningScreen::getRemainingTime() const {
    if (!_isRunning) return 0;
    
    uint32_t elapsed = getElapsedTime();
    if (elapsed >= _raceDuration) {
        return 0;  // Race finished
    }
    return _raceDuration - elapsed;
}

String LVGLRaceRunningScreen::formatTime(uint32_t milliseconds) const {
    uint32_t totalSeconds = milliseconds / 1000;
    uint32_t hours = totalSeconds / 3600;
    uint32_t minutes = (totalSeconds % 3600) / 60;
    uint32_t seconds = totalSeconds % 60;
    
    char buf[16];
    if (hours > 0) {
        snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", hours, minutes, seconds);
    } else {
        snprintf(buf, sizeof(buf), "%02lu:%02lu", minutes, seconds);
    }
    
    return String(buf);
}

void LVGLRaceRunningScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLRaceRunningScreen* screen = (LVGLRaceRunningScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLRaceRunning] Back button clicked");
        screen->_navigation->goBack();
    }
}

void LVGLRaceRunningScreen::pauseBtnEventHandler(lv_event_t* e) {
    LVGLRaceRunningScreen* screen = (LVGLRaceRunningScreen*)lv_event_get_user_data(e);
    if (screen) {
        screen->pauseRace();
        // Update button label
        if (screen->_isPaused) {
            lv_label_set_text(lv_obj_get_child(screen->_pauseButton, 0), "Weiter");
        } else {
            lv_label_set_text(lv_obj_get_child(screen->_pauseButton, 0), "Pause");
        }
    }
}

void LVGLRaceRunningScreen::stopBtnEventHandler(lv_event_t* e) {
    LVGLRaceRunningScreen* screen = (LVGLRaceRunningScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        Serial.println("[LVGLRaceRunning] Stop button clicked");
        screen->stopRace();
        // Navigation to results screen is handled in stopRace()
    }
}

