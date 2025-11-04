#ifndef THEME_H
#define THEME_H

#include <Arduino.h>
#include <lvgl.h>
#include "../core/config.h"

/**
 * UI Theme System
 * Unified design system with colors, spacing, typography, and icons
 */

// Color Palette (RGB565)
namespace Colors {
    // Primary Colors
    constexpr uint16_t PRIMARY = 0x3A9F;       // Vibrant blue
    constexpr uint16_t SECONDARY = 0x5FE0;     // Bright teal
    constexpr uint16_t ACCENT = 0xFD20;        // Orange accent

    // Semantic Colors
    constexpr uint16_t SUCCESS = 0x57E0;       // Green
    constexpr uint16_t WARNING = 0xFD00;       // Amber
    constexpr uint16_t DANGER = 0xF922;        // Red
    constexpr uint16_t INFO = 0x3A9F;          // Blue

    // UI Colors (high-contrast defaults)
    constexpr uint16_t BACKGROUND = 0x10A4;    // Dark slate
    constexpr uint16_t SURFACE = 0x1A86;       // Charcoal
    constexpr uint16_t SURFACE_ALT = 0x2967;   // Slightly lighter surface
    constexpr uint16_t TEXT = 0xFFFF;          // White
    constexpr uint16_t TEXT_SECONDARY = 0xAD55; // Soft grey

    // Component Colors
    constexpr uint16_t BUTTON = 0x2D6B;        // Steel
    constexpr uint16_t BUTTON_TEXT = 0xFFFF;   // White
    constexpr uint16_t BUTTON_PRESSED = 0x1CE7;// Darker steel
    constexpr uint16_t HEADER_BG = 0x001F;     // Deep blue
    constexpr uint16_t HEADER_TEXT = 0xFFFF;   // White

    // List Colors
    constexpr uint16_t LIST_ITEM = SURFACE_ALT;
    constexpr uint16_t LIST_ITEM_SELECTED = 0x5FE0; // Teal
    constexpr uint16_t LIST_ITEM_TEXT = TEXT;

    // Border
    constexpr uint16_t BORDER = 0x3186;        // Mid grey
}

// Typography
namespace Typography {
    constexpr uint8_t TINY = TEXT_SIZE_TINY;
    constexpr uint8_t SMALL = TEXT_SIZE_SMALL;
    constexpr uint8_t NORMAL = TEXT_SIZE_NORMAL;
    constexpr uint8_t LARGE = TEXT_SIZE_LARGE;
    constexpr uint8_t XLARGE = TEXT_SIZE_XLARGE;
}

namespace Fonts {
    enum class Size {
        Caption,
        Body,
        Subtitle,
        Title,
        Display
    };

    inline const lv_font_t* get(Size size) {
        switch (size) {
            case Size::Caption:
            #if LV_FONT_MONTSERRAT_12
                extern const lv_font_t lv_font_montserrat_12;
                return &lv_font_montserrat_12;
            #else
                return LV_FONT_DEFAULT;
            #endif
            case Size::Body:
            #if LV_FONT_MONTSERRAT_14
                extern const lv_font_t lv_font_montserrat_14;
                return &lv_font_montserrat_14;
            #else
                return LV_FONT_DEFAULT;
            #endif
            case Size::Subtitle:
            #if LV_FONT_MONTSERRAT_16
                extern const lv_font_t lv_font_montserrat_16;
                return &lv_font_montserrat_16;
            #else
                return LV_FONT_DEFAULT;
            #endif
            case Size::Title:
            #if LV_FONT_MONTSERRAT_20
                extern const lv_font_t lv_font_montserrat_20;
                return &lv_font_montserrat_20;
            #else
                return LV_FONT_DEFAULT;
            #endif
            case Size::Display:
            #if LV_FONT_MONTSERRAT_24
                extern const lv_font_t lv_font_montserrat_24;
                return &lv_font_montserrat_24;
            #else
                return LV_FONT_DEFAULT;
            #endif
        }
        return LV_FONT_DEFAULT;
    }
}

// Spacing (8px grid system)
namespace Spacing {
    constexpr uint8_t XS = GRID_UNIT / 2;      // 4px
    constexpr uint8_t SM = GRID_UNIT;          // 8px
    constexpr uint8_t MD = GRID_UNIT * 2;      // 16px
    constexpr uint8_t LG = GRID_UNIT * 3;      // 24px
    constexpr uint8_t XL = GRID_UNIT * 4;      // 32px
}

// Component Sizes
namespace Sizes {
    constexpr uint8_t BUTTON_H = BUTTON_HEIGHT;
    constexpr uint8_t LIST_ITEM_H = LIST_ITEM_HEIGHT;
    constexpr uint8_t HEADER_H = HEADER_HEIGHT;
    
    // Icon Sizes
    constexpr uint8_t ICON_SMALL = ICON_SIZE_SMALL;
    constexpr uint8_t ICON_MEDIUM = ICON_SIZE_MEDIUM;
    constexpr uint8_t ICON_LARGE = ICON_SIZE_LARGE;
    constexpr uint8_t ICON_XLARGE = ICON_SIZE_XLARGE;
}

// Icons (will be implemented as bitmap arrays)
// For now, define icon IDs
enum IconID {
    ICON_NONE = 0,
    ICON_HOME,
    ICON_SETTINGS,
    ICON_BACK,
    ICON_PLAY,
    ICON_PAUSE,
    ICON_STOP,
    ICON_TEAM,
    ICON_BEACON,
    ICON_RACE,
    ICON_RESULTS,
    ICON_UP,
    ICON_DOWN,
    ICON_CHECK,
    ICON_X,
    ICON_EDIT,
    ICON_ADD,
    ICON_DELETE,
    ICON_SAVE,
    ICON_CANCEL
};

#endif // THEME_H

