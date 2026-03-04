#include "lvgl_race_running_screen.h"
#include "../navigation_lvgl.h"
#include "../../services/data_logger_service.h"
#include <Arduino.h>

LVGLRaceRunningScreen::LVGLRaceRunningScreen(LVGLDisplay* lvglDisplay, BeaconService* beaconService, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _beaconService(beaconService)
    , _lapCounter(lapCounter)
    , _raceResultsScreen(nullptr)
    , _dataLogger(nullptr)
    , _showBoard(false)
    , _timeViewContainer(nullptr)
    , _bigTimeLabel(nullptr)
    , _bigRemainingLabel(nullptr)
    , _boardViewContainer(nullptr)
    , _smallTimeLabel(nullptr)
    , _leaderboardList(nullptr)
    , _timeLabel(nullptr)
    , _remainingLabel(nullptr)
    , _pauseButton(nullptr)
    , _stopButton(nullptr)
    , _viewButton(nullptr)
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
    
    _screen = _lvglDisplay->getScreen();
    if (!_screen) return;
    
    lv_obj_clean(_screen);
    _timeViewContainer = nullptr;
    _boardViewContainer = nullptr;
    _bigTimeLabel = nullptr;
    _bigRemainingLabel = nullptr;
    _smallTimeLabel = nullptr;
    _leaderboardList = nullptr;
    _pauseButton = nullptr;
    _stopButton = nullptr;
    _viewButton = nullptr;
    
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    createHeader("Race", false);
    
    // Layout: Header(35) | Content(171) | Buttons(34)
    int contentY = HEADER_HEIGHT;
    int btnH = 34;
    int btnY = SCREEN_HEIGHT - btnH;
    int contentH = btnY - contentY;
    
    // ===== TIME VIEW (default) =====
    _timeViewContainer = lv_obj_create(_screen);
    lv_obj_set_size(_timeViewContainer, SCREEN_WIDTH, contentH);
    lv_obj_set_pos(_timeViewContainer, 0, contentY);
    lv_obj_set_style_bg_opa(_timeViewContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_timeViewContainer, 0, 0);
    lv_obj_set_style_pad_all(_timeViewContainer, 0, 0);
    lv_obj_clear_flag(_timeViewContainer, LV_OBJ_FLAG_SCROLLABLE);
    
    // Big elapsed time - centered, largest font
    _bigTimeLabel = lv_label_create(_timeViewContainer);
    lv_label_set_text(_bigTimeLabel, "00:00");
    lv_obj_set_style_text_color(_bigTimeLabel, rgb565ToLVGL(Colors::TEXT), 0);
    extern const lv_font_t lv_font_montserrat_36;
    lv_obj_set_style_text_font(_bigTimeLabel, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_align(_bigTimeLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(_bigTimeLabel, SCREEN_WIDTH);
    lv_obj_align(_bigTimeLabel, LV_ALIGN_CENTER, 0, -25);
    
    // Remaining time below in bold-sized font
    _bigRemainingLabel = lv_label_create(_timeViewContainer);
    lv_label_set_text(_bigRemainingLabel, "-00:00");
    lv_obj_set_style_text_color(_bigRemainingLabel, rgb565ToLVGL(Colors::TEXT_SECONDARY), 0);
    extern const lv_font_t lv_font_montserrat_28;
    lv_obj_set_style_text_font(_bigRemainingLabel, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_align(_bigRemainingLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(_bigRemainingLabel, SCREEN_WIDTH);
    lv_obj_align(_bigRemainingLabel, LV_ALIGN_CENTER, 0, 25);
    
    // ===== BOARD VIEW (hidden initially) =====
    _boardViewContainer = lv_obj_create(_screen);
    lv_obj_set_size(_boardViewContainer, SCREEN_WIDTH, contentH);
    lv_obj_set_pos(_boardViewContainer, 0, contentY);
    lv_obj_set_style_bg_opa(_boardViewContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_boardViewContainer, 0, 0);
    lv_obj_set_style_pad_all(_boardViewContainer, 0, 0);
    lv_obj_clear_flag(_boardViewContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_boardViewContainer, LV_OBJ_FLAG_HIDDEN);
    
    // Time bar at top of board view
    _smallTimeLabel = lv_label_create(_boardViewContainer);
    lv_label_set_text(_smallTimeLabel, "00:00  -00:00");
    lv_obj_set_style_text_color(_smallTimeLabel, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_style_text_font(_smallTimeLabel, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(_smallTimeLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(_smallTimeLabel, SCREEN_WIDTH);
    lv_obj_set_pos(_smallTimeLabel, 0, 2);
    
    // Leaderboard list below the time bar
    int listY = 24;
    int listH = contentH - listY;
    _leaderboardList = lv_list_create(_boardViewContainer);
    lv_obj_set_size(_leaderboardList, SCREEN_WIDTH, listH);
    lv_obj_set_pos(_leaderboardList, 0, listY);
    lv_obj_set_style_bg_color(_leaderboardList, rgb565ToLVGL(Colors::BACKGROUND), 0);
    lv_obj_set_style_pad_all(_leaderboardList, 0, 0);
    lv_obj_set_style_pad_row(_leaderboardList, 1, 0);
    lv_obj_set_style_border_width(_leaderboardList, 0, 0);
    
    // ===== BUTTONS (always visible) =====
    int btnW = (SCREEN_WIDTH - 4 * Spacing::SM) / 3;
    
    _pauseButton = createButton("Pause", Spacing::SM, btnY, btnW, btnH, pauseBtnEventHandler, this);
    _stopButton = createButton("Stop", 2 * Spacing::SM + btnW, btnY, btnW, btnH, stopBtnEventHandler, this);
    if (_stopButton) {
        lv_obj_set_style_bg_color(_stopButton, rgb565ToLVGL(Colors::DANGER), 0);
    }
    _viewButton = createButton("Board", 3 * Spacing::SM + 2 * btnW, btnY, btnW, btnH, viewBtnEventHandler, this);
    if (_viewButton) {
        lv_obj_set_style_bg_color(_viewButton, rgb565ToLVGL(Colors::INFO), 0);
    }
    
    _showBoard = false;
    _lastUpdate = millis();
    _lastDisplayedTime = 0;
    _lastDisplayedRemaining = 0;
    
    if (_isRunning && _leaderboardList) {
        updateLeaderboard();
    }
    
    Serial.println("[LVGLRaceRunning] Race running screen created");
}

void LVGLRaceRunningScreen::onExit() {
    // Clean up race state without navigating (prevents infinite recursion)
    if (_isRunning) {
        _isRunning = false;
        _isPaused = false;
        if (_beaconService) {
            _beaconService->setRaceMode(false);
        }
        if (_lapCounter) _lapCounter->stopRace();
        if (_dataLogger) _dataLogger->logRaceEnd();
    }
}

void LVGLRaceRunningScreen::switchView() {
    _showBoard = !_showBoard;
    
    if (_showBoard) {
        lv_obj_add_flag(_timeViewContainer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_boardViewContainer, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(lv_obj_get_child(_viewButton, 0), "Time");
        updateLeaderboard();
    } else {
        lv_obj_clear_flag(_timeViewContainer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_boardViewContainer, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(lv_obj_get_child(_viewButton, 0), "Board");
    }
    
    // Force refresh of both time displays
    _lastDisplayedTime = 0;
    _lastDisplayedRemaining = 0;
}

void LVGLRaceRunningScreen::update() {
    if (!_isRunning) return;
    
    uint32_t remaining = getRemainingTime();
    if (remaining == 0 && !_isPaused) {
        Serial.println("[LVGLRaceRunning] Race time finished - auto stopping");
        stopRace();
        return;
    }
    
    uint32_t now = millis();
    if (now - _lastUpdate < 100) return;
    _lastUpdate = now;
    
    uint32_t elapsed = getElapsedTime();
    
    if (elapsed != _lastDisplayedTime) {
        String timeStr = formatTime(elapsed);
        
        if (_bigTimeLabel)
            lv_label_set_text(_bigTimeLabel, timeStr.c_str());
        
        if (_smallTimeLabel && _showBoard) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%s  -%s", timeStr.c_str(), formatTime(remaining).c_str());
            lv_label_set_text(_smallTimeLabel, buf);
        }
        
        _lastDisplayedTime = elapsed;
    }
    
    if (remaining != _lastDisplayedRemaining) {
        if (_bigRemainingLabel) {
            char buf[32];
            snprintf(buf, sizeof(buf), "-%s", formatTime(remaining).c_str());
            lv_label_set_text(_bigRemainingLabel, buf);
        }
        _lastDisplayedRemaining = remaining;
    }
    
    // Update leaderboard less frequently
    if (_showBoard) {
        static uint32_t lastLeaderboardUpdate = 0;
        if (now - lastLeaderboardUpdate > 500) {
            updateLeaderboard();
            lastLeaderboardUpdate = now;
        }
    }
}

void LVGLRaceRunningScreen::startRace(uint32_t durationMinutes, const char* raceName) {
    Serial.printf("[LVGLRaceRunning] startRace called - Duration: %lu minutes\n", durationMinutes);
    
    _raceStartTime = millis();
    _raceDuration = durationMinutes * 60 * 1000;
    _isRunning = true;
    _isPaused = false;
    _pausedTime = 0;
    _lastUpdate = millis();
    _lastDisplayedTime = 0;
    _lastDisplayedRemaining = 0;
    
    if (_beaconService) {
        _beaconService->setRaceMode(true);
        _beaconService->startScanning(0);
    }
    
    if (_lapCounter) {
        _lapCounter->startRace();
    }
    
    if (_dataLogger) {
        String name;
        if (raceName && strlen(raceName) > 0) {
            name = String(raceName);
            name.replace(" ", "_");
        }
        _dataLogger->logRaceStart(name);
    }
    
    Serial.printf("[LVGLRaceRunning] Race started - Duration: %lu minutes\n", durationMinutes);
}

void LVGLRaceRunningScreen::pauseRace() {
    if (!_isRunning) return;
    
    if (_isPaused) {
        _raceStartTime += (millis() - _pausedTime);
        _isPaused = false;
        Serial.println("[LVGLRaceRunning] Race resumed");
    } else {
        _pausedTime = millis();
        _isPaused = true;
        Serial.println("[LVGLRaceRunning] Race paused");
    }
}

void LVGLRaceRunningScreen::stopRace() {
    // onExit() handles cleanup; this just navigates to results
    Serial.println("[LVGLRaceRunning] Race stopped - navigating to results");
    if (_navigation && _raceResultsScreen) {
        _navigation->setScreen((LVGLBaseScreen*)_raceResultsScreen);
    }
}

void LVGLRaceRunningScreen::updateLeaderboard() {
    if (!_leaderboardList) return;
    
    lv_obj_clean(_leaderboardList);
    
    if (!_lapCounter) {
        lv_list_add_btn(_leaderboardList, NULL, "No LapCounter");
        return;
    }
    
    std::vector<TeamData*> leaderboard = _lapCounter->getLeaderboard();
    
    if (leaderboard.empty()) {
        lv_list_add_btn(_leaderboardList, NULL, "No teams");
        return;
    }
    
    int count = 0;
    for (TeamData* team : leaderboard) {
        if (count >= 10) break;
        
        char info[128];
        if (team->lapCount > 0 && team->bestLapTime > 0) {
            snprintf(info, sizeof(info), "%u. %s  %u laps  best:%lus", 
                    count + 1, team->teamName.c_str(), team->lapCount,
                    team->bestLapTime / 1000);
        } else {
            snprintf(info, sizeof(info), "%u. %s  %u laps", 
                    count + 1, team->teamName.c_str(), team->lapCount);
        }
        
        lv_obj_t* item = lv_list_add_btn(_leaderboardList, NULL, info);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        lv_obj_set_style_pad_ver(item, 1, 0);
        lv_obj_set_style_pad_hor(item, 4, 0);
        lv_obj_set_height(item, 22);
        lv_obj_set_style_min_height(item, 22, 0);
        lv_obj_set_style_max_height(item, 22, 0);
        
        count++;
    }
}

uint32_t LVGLRaceRunningScreen::getElapsedTime() const {
    if (!_isRunning) return 0;
    if (_isPaused) return _pausedTime - _raceStartTime;
    return millis() - _raceStartTime;
}

uint32_t LVGLRaceRunningScreen::getRemainingTime() const {
    if (!_isRunning) return 0;
    uint32_t elapsed = getElapsedTime();
    if (elapsed >= _raceDuration) return 0;
    return _raceDuration - elapsed;
}

String LVGLRaceRunningScreen::formatTime(uint32_t milliseconds) const {
    uint32_t totalSeconds = milliseconds / 1000;
    uint32_t hours = totalSeconds / 3600;
    uint32_t minutes = (totalSeconds % 3600) / 60;
    uint32_t seconds = totalSeconds % 60;
    
    char buf[16];
    if (hours > 0) {
        snprintf(buf, sizeof(buf), "%lu:%02lu:%02lu", hours, minutes, seconds);
    } else {
        snprintf(buf, sizeof(buf), "%02lu:%02lu", minutes, seconds);
    }
    return String(buf);
}

void LVGLRaceRunningScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLRaceRunningScreen* screen = (LVGLRaceRunningScreen*)lv_event_get_user_data(e);
    if (screen && screen->_navigation) {
        screen->_navigation->goBack();
    }
}

void LVGLRaceRunningScreen::pauseBtnEventHandler(lv_event_t* e) {
    LVGLRaceRunningScreen* screen = (LVGLRaceRunningScreen*)lv_event_get_user_data(e);
    if (screen) {
        screen->pauseRace();
        if (screen->_pauseButton) {
            lv_label_set_text(lv_obj_get_child(screen->_pauseButton, 0),
                            screen->_isPaused ? "Resume" : "Pause");
        }
    }
}

void LVGLRaceRunningScreen::stopBtnEventHandler(lv_event_t* e) {
    LVGLRaceRunningScreen* screen = (LVGLRaceRunningScreen*)lv_event_get_user_data(e);
    if (screen) {
        screen->stopRace();
    }
}

void LVGLRaceRunningScreen::viewBtnEventHandler(lv_event_t* e) {
    LVGLRaceRunningScreen* screen = (LVGLRaceRunningScreen*)lv_event_get_user_data(e);
    if (screen) {
        screen->switchView();
    }
}
