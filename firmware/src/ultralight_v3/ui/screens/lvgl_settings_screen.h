#ifndef LVGL_SETTINGS_SCREEN_H
#define LVGL_SETTINGS_SCREEN_H

#include "lvgl_base_screen.h"
#include <lvgl.h>

// Forward declarations
class LVGLNavigation;
class PersistenceService;
class DataLoggerService;
class LapCounterService;

/**
 * Settings Screen (LVGL8)
 * System settings and configuration
 */
class LVGLSettingsScreen : public LVGLBaseScreen {
public:
    LVGLSettingsScreen(LVGLDisplay* lvglDisplay, PersistenceService* persistence = nullptr, DataLoggerService* dataLogger = nullptr, LapCounterService* lapCounter = nullptr);
    virtual ~LVGLSettingsScreen();
    
    void onEnter() override;
    void onExit() override;
    void update() override {}
    
    // Set navigation for back button
    void setNavigation(LVGLNavigation* nav) { _navigation = nav; }
    
private:
    LVGLNavigation* _navigation;
    PersistenceService* _persistence;
    DataLoggerService* _dataLogger;
    LapCounterService* _lapCounter;
    lv_obj_t* _list;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    static void settingsItemEventHandler(lv_event_t* e);
    
    // Helper methods
    void createSettingsList();
    void handleBLESettings();
    void handleSaveData();
    void handleReset();
};

#endif // LVGL_SETTINGS_SCREEN_H

