#ifndef LVGL_SETTINGS_SCREEN_H
#define LVGL_SETTINGS_SCREEN_H

#include "lvgl_base_screen.h"
#include "../widgets/lvgl_dialog.h"
#include <lvgl.h>

// Forward declarations
class LVGLNavigation;
class PersistenceService;
class DataLoggerService;
class LapCounterService;
class BeaconService;

/**
 * Settings Screen (LVGL8)
 * System settings and configuration
 */
class LVGLSettingsScreen : public LVGLBaseScreen {
public:
    LVGLSettingsScreen(LVGLDisplay* lvglDisplay,
                      PersistenceService* persistence = nullptr,
                      DataLoggerService* dataLogger = nullptr,
                      LapCounterService* lapCounter = nullptr,
                      BeaconService* beaconService = nullptr);
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
    BeaconService* _beaconService;
    lv_obj_t* _list;
    LVGLDialog* _dialog;
    lv_obj_t* _bleDialog;
    lv_obj_t* _bleSlider;
    lv_obj_t* _bleValueLabel;
    lv_obj_t* _bleScanSwitch;
    
    // Event handlers
    static void backBtnEventHandler(lv_event_t* e);
    static void settingsItemEventHandler(lv_event_t* e);
    static void bleDialogCloseHandler(lv_event_t* e);
    static void bleThresholdChangedHandler(lv_event_t* e);
    static void bleScanToggleHandler(lv_event_t* e);
    
    // Helper methods
    void createSettingsList();
    void handleBLESettings();
    void handleSaveData();
    void handleReset();
    void closeBLEDialog();
    void updateBLEValueLabel(int16_t threshold);
};

#endif // LVGL_SETTINGS_SCREEN_H

