#ifndef LVGL_SETTINGS_SCREEN_H
#define LVGL_SETTINGS_SCREEN_H

#include "lvgl_base_screen.h"
#include <lvgl.h>

// Forward declarations
class LVGLNavigation;
class PersistenceService;
class DataLoggerService;
class LapCounterService;
class LVGLDebugScreen;

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
    void setDebugScreen(LVGLDebugScreen* screen);
    
private:
    LVGLNavigation* _navigation;
    PersistenceService* _persistence;
    DataLoggerService* _dataLogger;
    LapCounterService* _lapCounter;
    LVGLDebugScreen* _debugScreen;
    lv_obj_t* _list;
    
    lv_obj_t* _minLapTimeItem;
    lv_obj_t* _rssiNearItem;
    lv_obj_t* _rssiFarItem;
    
    // Value editor modal
    lv_obj_t* _editModal;
    lv_obj_t* _editValueLabel;
    int32_t _editValue;
    int32_t _editMin;
    int32_t _editMax;
    int32_t _editStep;
    uint8_t _editKey;  // 0=minLap, 1=rssiNear, 2=rssiFar
    
    static void backBtnEventHandler(lv_event_t* e);
    static void settingsItemEventHandler(lv_event_t* e);
    static void resetConfirmHandler(lv_event_t* e);
    static void minLapTimeEventHandler(lv_event_t* e);
    static void rssiNearEventHandler(lv_event_t* e);
    static void rssiFarEventHandler(lv_event_t* e);
    static void editMinusHandler(lv_event_t* e);
    static void editPlusHandler(lv_event_t* e);
    static void editOkHandler(lv_event_t* e);
    
    void createSettingsList();
    void handleBLESettings();
    void handleSaveData();
    void handleReset();
    void updateMinLapTimeLabel();
    void updateRssiNearLabel();
    void updateRssiFarLabel();
    void showValueEditor(const char* title, int32_t current, int32_t min, int32_t max, int32_t step, uint8_t key);
    void updateEditDisplay();
    void closeEditor();
    void saveEditedValue();
};

#endif // LVGL_SETTINGS_SCREEN_H

