#ifndef ICONS_H
#define ICONS_H

#include <Arduino.h>
#include <lvgl.h>
#include "../theme.h"

/**
 * Icon System
 * Bitmap-based icons for LVGL8
 * Icons are stored as RGB565 bitmaps (16x16, 24x24, 32x32)
 */

// Icon structure for LVGL
struct Icon {
    const uint16_t* data;  // RGB565 pixel data
    uint16_t width;
    uint16_t height;
};

// Icon bitmaps (16x16 RGB565)
// Simple geometric shapes for now - can be replaced with proper icons later

extern const uint16_t icon_home_16[];
extern const uint16_t icon_settings_16[];
extern const uint16_t icon_back_16[];
extern const uint16_t icon_play_16[];
extern const uint16_t icon_pause_16[];
extern const uint16_t icon_stop_16[];
extern const uint16_t icon_team_16[];
extern const uint16_t icon_beacon_16[];
extern const uint16_t icon_race_16[];
extern const uint16_t icon_results_16[];
extern const uint16_t icon_up_16[];
extern const uint16_t icon_down_16[];
extern const uint16_t icon_check_16[];
extern const uint16_t icon_x_16[];
extern const uint16_t icon_edit_16[];
extern const uint16_t icon_add_16[];
extern const uint16_t icon_delete_16[];
extern const uint16_t icon_save_16[];
extern const uint16_t icon_cancel_16[];

// Icon registry
const Icon* getIcon(IconID id, uint8_t size = 16);

// Helper function to create LVGL image from icon
lv_obj_t* createIconImage(lv_obj_t* parent, IconID iconId, uint8_t size = 16);

#endif // ICONS_H

