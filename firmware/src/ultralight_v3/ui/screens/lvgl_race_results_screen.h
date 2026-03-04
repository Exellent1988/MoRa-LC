#ifndef LVGL_RACE_RESULTS_SCREEN_H
#define LVGL_RACE_RESULTS_SCREEN_H

#include "lvgl_base_screen.h"
#include "../../services/lap_counter_service.h"
#include "../../services/data_logger_service.h"
#include <lvgl.h>
#include <vector>
#include <map>

class LVGLNavigation;

class LVGLRaceResultsScreen : public LVGLBaseScreen {
public:
    LVGLRaceResultsScreen(LVGLDisplay* lvglDisplay, LapCounterService* lapCounter);
    virtual ~LVGLRaceResultsScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    void setHomeScreen(LVGLBaseScreen* home) { _homeScreen = home; }
    void setDataLogger(DataLoggerService* dl) { _dataLogger = dl; }
    
private:
    LVGLNavigation* _navigation;
    LVGLBaseScreen* _homeScreen;
    LapCounterService* _lapCounter;
    DataLoggerService* _dataLogger;
    lv_obj_t* _list;
    
    // View state: 0=race list, 1=race detail, 2=team laps
    uint8_t _viewLevel;
    std::vector<String> _raceFiles;
    String _currentFilePath;
    String _currentCSVContent;
    uint8_t _selectedTeamId;
    
    struct ParsedTeam {
        uint8_t teamId;
        String teamName;
        uint32_t lapCount;
        uint32_t bestLapMs;
    };
    
    struct LapEntry {
        uint32_t lapNumber;
        uint32_t durationMs;
        String timeOfDay;
        int8_t rssi;
    };
    
    static void backBtnEventHandler(lv_event_t* e);
    static void raceItemEventHandler(lv_event_t* e);
    static void teamItemEventHandler(lv_event_t* e);
    
    void showRaceList();
    void showRaceDetail(const String& filePath);
    void showCurrentRaceResults();
    void showTeamLaps(uint8_t teamId, const String& teamName);
    String extractRaceName(const String& filePath);
    
    std::vector<ParsedTeam> parseCSV(const String& content);
    std::vector<LapEntry> parseLapsForTeam(const String& content, uint8_t teamId);
};

#endif // LVGL_RACE_RESULTS_SCREEN_H
