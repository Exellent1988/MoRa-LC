#include "lvgl_race_results_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLRaceResultsScreen::LVGLRaceResultsScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _homeScreen(nullptr)
    , _lapCounter(lapCounter)
    , _dataLogger(nullptr)
    , _list(nullptr)
    , _viewLevel(0)
    , _selectedTeamId(0) {
}

LVGLRaceResultsScreen::~LVGLRaceResultsScreen() {
}

void LVGLRaceResultsScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) return;
    _screen = _lvglDisplay->getScreen();
    _viewLevel = 0;
    _currentCSVContent = "";
    _currentFilePath = "";
    showRaceList();
}

void LVGLRaceResultsScreen::onExit() {
    _currentCSVContent = "";
}

// ── Race List (level 0) ────────────────────────────────

void LVGLRaceResultsScreen::showRaceList() {
    _viewLevel = 0;
    lv_obj_clean(_screen);
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    createHeader("Results", true, backBtnEventHandler, this);
    
    int listY = HEADER_HEIGHT + 2;
    _list = createList(0, listY, SCREEN_WIDTH, SCREEN_HEIGHT - listY);
    
    // Current race (in-memory)
    bool hasCurrentRace = false;
    if (_lapCounter) {
        for (auto* t : _lapCounter->getLeaderboard()) {
            if (t->lapCount > 0) { hasCurrentRace = true; break; }
        }
    }
    if (hasCurrentRace) {
        lv_obj_t* item = lv_list_add_btn(_list, LV_SYMBOL_PLAY, "Current Race");
        lv_obj_set_height(item, 32);
        lv_obj_set_style_pad_ver(item, 4, 0);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SECONDARY), 0);
        lv_obj_set_user_data(item, (void*)(uintptr_t)0xFFFF);
        lv_obj_add_event_cb(item, raceItemEventHandler, LV_EVENT_SHORT_CLICKED, this);
    }
    
    // Historical races from SD
    _raceFiles.clear();
    if (_dataLogger && _dataLogger->isReady()) {
        _raceFiles = _dataLogger->getRaceFiles(10);
    }
    
    if (_raceFiles.empty() && !hasCurrentRace) {
        lv_obj_t* empty = lv_list_add_btn(_list, NULL, "No races found");
        lv_obj_set_style_bg_color(empty, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    for (size_t i = 0; i < _raceFiles.size(); i++) {
        String label = extractRaceName(_raceFiles[i]);
        lv_obj_t* item = lv_list_add_btn(_list, LV_SYMBOL_FILE, label.c_str());
        lv_obj_set_height(item, 30);
        lv_obj_set_style_pad_ver(item, 3, 0);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        lv_obj_set_user_data(item, (void*)(uintptr_t)i);
        lv_obj_add_event_cb(item, raceItemEventHandler, LV_EVENT_SHORT_CLICKED, this);
    }
}

// ── Race Detail (level 1) ──────────────────────────────

void LVGLRaceResultsScreen::showCurrentRaceResults() {
    _viewLevel = 1;
    _currentFilePath = "";
    _currentCSVContent = "";
    lv_obj_clean(_screen);
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    createHeader("Current Race", true, backBtnEventHandler, this);
    
    int listY = HEADER_HEIGHT + 2;
    _list = createList(0, listY, SCREEN_WIDTH, SCREEN_HEIGHT - listY);
    
    if (!_lapCounter) return;
    
    auto leaderboard = _lapCounter->getLeaderboard();
    int rank = 1;
    for (TeamData* team : leaderboard) {
        char info[96];
        if (team->bestLapTime > 0) {
            uint32_t sec = team->bestLapTime / 1000;
            uint32_t min = sec / 60; sec %= 60;
            snprintf(info, sizeof(info), "%d. %s - %u Laps, Best %lu:%02lu",
                     rank, team->teamName.c_str(), team->lapCount, min, sec);
        } else {
            snprintf(info, sizeof(info), "%d. %s - %u Laps",
                     rank, team->teamName.c_str(), team->lapCount);
        }
        lv_obj_t* item = lv_list_add_btn(_list, NULL, info);
        lv_obj_set_height(item, 28);
        lv_obj_set_style_pad_ver(item, 3, 0);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        rank++;
    }
}

void LVGLRaceResultsScreen::showRaceDetail(const String& filePath) {
    _viewLevel = 1;
    _currentFilePath = filePath;
    lv_obj_clean(_screen);
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    String title = extractRaceName(filePath);
    createHeader(title.c_str(), true, backBtnEventHandler, this);
    
    int listY = HEADER_HEIGHT + 2;
    _list = createList(0, listY, SCREEN_WIDTH, SCREEN_HEIGHT - listY);
    
    if (!_dataLogger) return;
    
    _currentCSVContent = _dataLogger->readFile(filePath);
    if (_currentCSVContent.length() == 0) {
        lv_obj_t* item = lv_list_add_btn(_list, NULL, "Could not read file");
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    auto teams = parseCSV(_currentCSVContent);
    if (teams.empty()) {
        lv_obj_t* item = lv_list_add_btn(_list, NULL, "No lap data");
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    // Sort by lap count descending
    for (size_t i = 0; i < teams.size(); i++) {
        for (size_t j = i + 1; j < teams.size(); j++) {
            if (teams[j].lapCount > teams[i].lapCount) {
                ParsedTeam tmp = teams[i]; teams[i] = teams[j]; teams[j] = tmp;
            }
        }
    }
    
    int rank = 1;
    for (const auto& t : teams) {
        char info[96];
        if (t.bestLapMs > 0) {
            uint32_t sec = t.bestLapMs / 1000;
            uint32_t min = sec / 60; sec %= 60;
            snprintf(info, sizeof(info), "%d. %s - %lu Laps, Best %lu:%02lu",
                     rank, t.teamName.c_str(), t.lapCount, min, sec);
        } else {
            snprintf(info, sizeof(info), "%d. %s - %lu Laps",
                     rank, t.teamName.c_str(), t.lapCount);
        }
        lv_obj_t* item = lv_list_add_btn(_list, LV_SYMBOL_RIGHT, info);
        lv_obj_set_height(item, 28);
        lv_obj_set_style_pad_ver(item, 3, 0);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        lv_obj_set_user_data(item, (void*)(uintptr_t)t.teamId);
        lv_obj_add_event_cb(item, teamItemEventHandler, LV_EVENT_SHORT_CLICKED, this);
        rank++;
    }
}

// ── Team Laps (level 2) ────────────────────────────────

void LVGLRaceResultsScreen::showTeamLaps(uint8_t teamId, const String& teamName) {
    _viewLevel = 2;
    _selectedTeamId = teamId;
    lv_obj_clean(_screen);
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    createHeader(teamName.c_str(), true, backBtnEventHandler, this);
    
    int listY = HEADER_HEIGHT + 2;
    _list = createList(0, listY, SCREEN_WIDTH, SCREEN_HEIGHT - listY);
    
    auto laps = parseLapsForTeam(_currentCSVContent, teamId);
    
    if (laps.empty()) {
        lv_obj_t* item = lv_list_add_btn(_list, NULL, "No laps");
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
        return;
    }
    
    for (const auto& lap : laps) {
        char info[80];
        uint32_t sec = lap.durationMs / 1000;
        uint32_t min = sec / 60; sec %= 60;
        uint32_t ms = lap.durationMs % 1000;
        
        snprintf(info, sizeof(info), "Lap %lu: %lu:%02lu.%03lu  %s  %ddBm",
                 lap.lapNumber, min, sec, ms,
                 lap.timeOfDay.c_str(), lap.rssi);
        
        lv_obj_t* item = lv_list_add_btn(_list, NULL, info);
        lv_obj_set_height(item, 26);
        lv_obj_set_style_pad_ver(item, 2, 0);
        lv_obj_set_style_bg_color(item, rgb565ToLVGL(Colors::SURFACE), 0);
    }
}

// ── CSV Parsing ────────────────────────────────────────

std::vector<LVGLRaceResultsScreen::ParsedTeam> LVGLRaceResultsScreen::parseCSV(const String& content) {
    std::map<uint8_t, ParsedTeam> teamMap;
    int lineStart = 0;
    bool headerSkipped = false;
    
    while (lineStart < (int)content.length()) {
        int lineEnd = content.indexOf('\n', lineStart);
        if (lineEnd < 0) lineEnd = content.length();
        String line = content.substring(lineStart, lineEnd);
        line.trim();
        lineStart = lineEnd + 1;
        if (line.length() == 0) continue;
        if (!headerSkipped) { headerSkipped = true; continue; }
        
        // Team ID,Team Name,Lap Number,Timestamp,Duration,Time of Day,RSSI
        int c1 = line.indexOf(',');
        if (c1 < 0) continue;
        int c2 = line.indexOf(',', c1 + 1);
        if (c2 < 0) continue;
        int c3 = line.indexOf(',', c2 + 1);
        if (c3 < 0) continue;
        int c4 = line.indexOf(',', c3 + 1);
        if (c4 < 0) continue;
        int c5 = line.indexOf(',', c4 + 1);
        
        uint8_t teamId = (uint8_t)line.substring(0, c1).toInt();
        String teamName = line.substring(c1 + 1, c2);
        uint32_t lapNum = (uint32_t)line.substring(c2 + 1, c3).toInt();
        uint32_t duration = 0;
        if (c5 > c4) duration = (uint32_t)line.substring(c4 + 1, c5).toInt();
        
        auto it = teamMap.find(teamId);
        if (it == teamMap.end()) {
            ParsedTeam pt;
            pt.teamId = teamId;
            pt.teamName = teamName;
            pt.lapCount = lapNum;
            pt.bestLapMs = duration;
            teamMap[teamId] = pt;
        } else {
            if (lapNum > it->second.lapCount) it->second.lapCount = lapNum;
            if (duration > 0 && (it->second.bestLapMs == 0 || duration < it->second.bestLapMs))
                it->second.bestLapMs = duration;
        }
    }
    
    std::vector<ParsedTeam> result;
    for (auto& pair : teamMap) result.push_back(pair.second);
    return result;
}

std::vector<LVGLRaceResultsScreen::LapEntry> LVGLRaceResultsScreen::parseLapsForTeam(const String& content, uint8_t teamId) {
    std::vector<LapEntry> laps;
    int lineStart = 0;
    bool headerSkipped = false;
    
    while (lineStart < (int)content.length()) {
        int lineEnd = content.indexOf('\n', lineStart);
        if (lineEnd < 0) lineEnd = content.length();
        String line = content.substring(lineStart, lineEnd);
        line.trim();
        lineStart = lineEnd + 1;
        if (line.length() == 0) continue;
        if (!headerSkipped) { headerSkipped = true; continue; }
        
        // Team ID,Team Name,Lap Number,Timestamp,Duration,Time of Day,RSSI
        int c1 = line.indexOf(',');
        if (c1 < 0) continue;
        int c2 = line.indexOf(',', c1 + 1);
        if (c2 < 0) continue;
        int c3 = line.indexOf(',', c2 + 1);
        if (c3 < 0) continue;
        int c4 = line.indexOf(',', c3 + 1);
        if (c4 < 0) continue;
        int c5 = line.indexOf(',', c4 + 1);
        if (c5 < 0) continue;
        int c6 = line.indexOf(',', c5 + 1);
        
        uint8_t id = (uint8_t)line.substring(0, c1).toInt();
        if (id != teamId) continue;
        
        LapEntry lap;
        lap.lapNumber = (uint32_t)line.substring(c2 + 1, c3).toInt();
        lap.durationMs = (uint32_t)line.substring(c4 + 1, c5).toInt();
        lap.timeOfDay = line.substring(c5 + 1, (c6 > c5) ? c6 : line.length());
        lap.rssi = (c6 > c5) ? (int8_t)line.substring(c6 + 1).toInt() : 0;
        laps.push_back(lap);
    }
    return laps;
}

// ── Navigation & Helpers ───────────────────────────────

String LVGLRaceResultsScreen::extractRaceName(const String& filePath) {
    int lastSlash = filePath.lastIndexOf('/');
    String name = (lastSlash >= 0) ? filePath.substring(lastSlash + 1) : filePath;
    if (name.endsWith(".csv")) name = name.substring(0, name.length() - 4);
    if (name.startsWith("race_")) name = name.substring(5);
    
    // If name contains underscore after the millis number, use that as display name
    int uscore = name.indexOf('_');
    if (uscore > 0) {
        String label = name.substring(uscore + 1);
        label.replace("_", " ");
        return label;
    }
    
    return "Race " + name;
}

void LVGLRaceResultsScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLRaceResultsScreen* s = (LVGLRaceResultsScreen*)lv_event_get_user_data(e);
    if (!s || !s->_navigation) return;
    
    switch (s->_viewLevel) {
        case 2:  // Team laps -> back to race detail
            if (s->_currentFilePath.length() > 0) {
                s->showRaceDetail(s->_currentFilePath);
            } else {
                s->showCurrentRaceResults();
            }
            break;
        case 1:  // Race detail -> back to race list
            s->showRaceList();
            break;
        default: // Race list -> home
            if (s->_homeScreen) {
                s->_navigation->setScreen(s->_homeScreen);
            } else {
                s->_navigation->goBack();
            }
            break;
    }
}

void LVGLRaceResultsScreen::raceItemEventHandler(lv_event_t* e) {
    LVGLRaceResultsScreen* s = (LVGLRaceResultsScreen*)lv_event_get_user_data(e);
    if (!s) return;
    uintptr_t idx = (uintptr_t)lv_obj_get_user_data(lv_event_get_target(e));
    
    if (idx == 0xFFFF) {
        s->showCurrentRaceResults();
    } else if (idx < s->_raceFiles.size()) {
        s->showRaceDetail(s->_raceFiles[idx]);
    }
}

void LVGLRaceResultsScreen::teamItemEventHandler(lv_event_t* e) {
    LVGLRaceResultsScreen* s = (LVGLRaceResultsScreen*)lv_event_get_user_data(e);
    if (!s) return;
    
    uint8_t teamId = (uint8_t)(uintptr_t)lv_obj_get_user_data(lv_event_get_target(e));
    
    // Find team name from CSV
    auto teams = s->parseCSV(s->_currentCSVContent);
    String teamName = "Team";
    for (const auto& t : teams) {
        if (t.teamId == teamId) { teamName = t.teamName; break; }
    }
    
    s->showTeamLaps(teamId, teamName);
}
