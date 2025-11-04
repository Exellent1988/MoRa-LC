#include "icons.h"
#include <lvgl.h>

// Placeholder icon data (16x16 RGB565) - simple colored squares
// TODO: Replace with proper icon bitmaps
const uint16_t icon_home_16[256] = {0}; // 16x16 = 256 pixels
const uint16_t icon_settings_16[256] = {0};
const uint16_t icon_back_16[256] = {0};
const uint16_t icon_play_16[256] = {0};
const uint16_t icon_pause_16[256] = {0};
const uint16_t icon_stop_16[256] = {0};
const uint16_t icon_team_16[256] = {0};
const uint16_t icon_beacon_16[256] = {0};
const uint16_t icon_race_16[256] = {0};
const uint16_t icon_results_16[256] = {0};
const uint16_t icon_up_16[256] = {0};
const uint16_t icon_down_16[256] = {0};
const uint16_t icon_check_16[256] = {0};
const uint16_t icon_x_16[256] = {0};
const uint16_t icon_edit_16[256] = {0};
const uint16_t icon_add_16[256] = {0};
const uint16_t icon_delete_16[256] = {0};
const uint16_t icon_save_16[256] = {0};
const uint16_t icon_cancel_16[256] = {0};

// Icon registry
static const Icon icons_16[] = {
    {icon_home_16, 16, 16},
    {icon_settings_16, 16, 16},
    {icon_back_16, 16, 16},
    {icon_play_16, 16, 16},
    {icon_pause_16, 16, 16},
    {icon_stop_16, 16, 16},
    {icon_team_16, 16, 16},
    {icon_beacon_16, 16, 16},
    {icon_race_16, 16, 16},
    {icon_results_16, 16, 16},
    {icon_up_16, 16, 16},
    {icon_down_16, 16, 16},
    {icon_check_16, 16, 16},
    {icon_x_16, 16, 16},
    {icon_edit_16, 16, 16},
    {icon_add_16, 16, 16},
    {icon_delete_16, 16, 16},
    {icon_save_16, 16, 16},
    {icon_cancel_16, 16, 16}
};

const Icon* getIcon(IconID id, uint8_t size) {
    if (id == ICON_NONE || id > ICON_CANCEL) return nullptr;
    if (size == 16 && id <= ICON_CANCEL) {
        return &icons_16[id - 1]; // IDs start at 1
    }
    return nullptr; // Other sizes not implemented yet
}

// Helper function to create LVGL image from icon
lv_obj_t* createIconImage(lv_obj_t* parent, IconID iconId, uint8_t size) {
    const Icon* icon = getIcon(iconId, size);
    if (!icon || !icon->data) return nullptr;
    
    // Create image descriptor for LVGL
    lv_img_dsc_t img_dsc;
    img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    img_dsc.header.w = icon->width;
    img_dsc.header.h = icon->height;
    img_dsc.data_size = icon->width * icon->height * 2; // RGB565 = 2 bytes per pixel
    img_dsc.data = (const uint8_t*)icon->data;
    
    // Create LVGL image object
    lv_obj_t* img = lv_img_create(parent);
    lv_img_set_src(img, &img_dsc);
    
    return img;
}

