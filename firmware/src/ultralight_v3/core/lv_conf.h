/**
 * @file lv_conf.h
 * LVGL8 Configuration for MoRa-LC UltraLight v3
 * Based on ESP32-Cheap-Yellow-Display example
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*Color depth: 16 = RGB565 (matches LovyanGFX)*/
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/*Memory pool size (32KB for ESP32)*/
#define LV_MEM_SIZE (32U * 1024U)

/*Tick source - use Arduino millis()*/
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

/*DPI (affects default sizes)*/
#define LV_DPI_DEF 130

/*Log level - keep minimal for performance*/
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1

/*Enable essential widgets (add more as needed)*/
#define LV_USE_LABEL 1
#define LV_USE_BUTTON 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_BAR 1
#define LV_USE_IMG 1
#define LV_USE_LIST 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1

/*Font settings - use built-in fonts*/
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*Enable themes*/
#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_BASIC 1

/*Enable animations*/
#define LV_USE_ANIMATION 1

/*Enable layout systems*/
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*Disable unused features to save memory*/
#define LV_USE_FS_STDIO 0
#define LV_USE_PNG 0
#define LV_USE_BMP 0
#define LV_USE_SJPG 0
#define LV_USE_FREETYPE 0

/*Assertions - keep minimal for performance*/
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

#endif /*LV_CONF_H*/
