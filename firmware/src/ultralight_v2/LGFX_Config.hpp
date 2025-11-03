// LovyanGFX Configuration for ESP32-2432S028
// ST7789 Display with XPT2046 Touch

#pragma once

#include "config.h"

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
public:
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Touch_XPT2046 _touch_instance;

    LGFX(void)
    {
        // SPI Bus Configuration for Display
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = VSPI_HOST;          // Use VSPI
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;         // 40 MHz
            cfg.freq_read = 20000000;          // 20 MHz
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            cfg.dma_channel = 1;
            cfg.pin_sclk = TFT_SCLK_PIN;       // 14
            cfg.pin_mosi = TFT_MOSI_PIN;       // 13
            cfg.pin_miso = TFT_MISO_PIN;       // 12
            cfg.pin_dc = TFT_DC_PIN;           // 2
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // Panel Configuration (ST7789)
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = TFT_CS_PIN;           // 15
            cfg.pin_rst = TFT_RST_PIN;         // -1 (not used)
            cfg.pin_busy = -1;
            // ST7789 physical dimensions (native portrait)
            cfg.memory_width = 240;
            cfg.memory_height = 320;
            // Panel dimensions must match physical hardware (240x320 portrait)
            // Rotation is handled separately via setRotation()
            cfg.panel_width = 240;             // Physical width (portrait)
            cfg.panel_height = 320;            // Physical height (portrait)
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = true;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;            // Display has its own SPI bus
            _panel_instance.config(cfg);
        }

        // Touch Configuration (XPT2046)
        {
            auto cfg = _touch_instance.config();
            // XPT2046 Raw value ranges (0-4095, typical 200-3800)
            // These are the raw ADC values from the touch controller
            // Note: X and Y might need to be swapped depending on hardware orientation
            cfg.x_min = 200;
            cfg.x_max = 3700;
            cfg.y_min = 200;
            cfg.y_max = 3800;
            cfg.pin_int = TOUCH_IRQ_PIN;       // 36 (optional)
            cfg.bus_shared = false;            // Touch has separate SPI bus
            cfg.offset_rotation = 0;           // Touch rotation offset (0, 1, 2, or 3 for 0°, 90°, 180°, 270°)
            cfg.spi_host = HSPI_HOST;          // Use HSPI for Touch (separate from Display)
            cfg.freq = 2500000;                 // 2.5 MHz
            cfg.pin_sclk = TOUCH_CLK_PIN;      // 25
            cfg.pin_mosi = TOUCH_MOSI_PIN;     // 32
            cfg.pin_miso = TOUCH_MISO_PIN;     // 39
            cfg.pin_cs = TOUCH_CS_PIN;         // 33
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

