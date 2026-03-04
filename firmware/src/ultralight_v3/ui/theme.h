#ifndef THEME_H
#define THEME_H

#include <Arduino.h>
#include "../core/config.h"

/**
 * UI Theme System
 * Unified design system with colors, spacing, typography, and icons
 *
 * LVGL color mapping (ESP32-2432S028 + LV_COLOR_16_SWAP=1, rgb_order=true):
 * Tested via color.full assignment through rgb565ToLVGL():
 *   0xF81F → blue       0x07FF → bright red    0xFFE0 → green
 *   0x07E0 → lime       0xF800 → dark red      0x001F → purple
 *   0xFFFF → white      0x0000 → black
 * Grays (8410, CE59, 4208) show with blue tint - may need tuning.
 */

// Color Palette (RGB565 via color.full - empirically calibrated)
namespace Colors {
    // Primary Colors
    constexpr uint16_t PRIMARY = 0xF81F;      // Blue (on this display)
    constexpr uint16_t SECONDARY = 0xFFE0;    // Green
    constexpr uint16_t ACCENT = 0x07E0;       // Lime/Yellow-Green
    
    // Semantic Colors
    constexpr uint16_t SUCCESS = 0xFFE0;      // Green
    constexpr uint16_t WARNING = 0xF800;      // Dark Red / Orange-ish
    constexpr uint16_t DANGER = 0x07FF;       // Bright Red
    constexpr uint16_t INFO = 0xF81F;         // Blue
    
    // UI Colors
    constexpr uint16_t BACKGROUND = 0xC618;   // Neutral Light Gray
    constexpr uint16_t SURFACE = 0xFFFF;      // White
    constexpr uint16_t TEXT = 0x0000;          // Black
    constexpr uint16_t TEXT_SECONDARY = 0x4204; // Dark Gray (reduced blue)
    
    // Component Colors
    constexpr uint16_t BUTTON = 0x8408;        // Neutral Medium Gray
    constexpr uint16_t BUTTON_TEXT = 0xFFFF;   // White
    constexpr uint16_t BUTTON_PRESSED = 0x6300; // Darker Gray (reduced blue)
    constexpr uint16_t HEADER_BG = 0xF81F;     // Blue
    constexpr uint16_t HEADER_TEXT = 0xFFFF;   // White
    
    // List Colors
    constexpr uint16_t LIST_ITEM = 0x8408;     // Neutral Medium Gray
    constexpr uint16_t LIST_ITEM_SELECTED = 0xFFE0; // Green
    constexpr uint16_t LIST_ITEM_TEXT = 0x0000; // Black
    
    // Border
    constexpr uint16_t BORDER = 0x4204;        // Dark Gray (reduced blue)
}

// Typography
namespace Typography {
    constexpr uint8_t TINY = TEXT_SIZE_TINY;
    constexpr uint8_t SMALL = TEXT_SIZE_SMALL;
    constexpr uint8_t NORMAL = TEXT_SIZE_NORMAL;
    constexpr uint8_t LARGE = TEXT_SIZE_LARGE;
    constexpr uint8_t XLARGE = TEXT_SIZE_XLARGE;
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

