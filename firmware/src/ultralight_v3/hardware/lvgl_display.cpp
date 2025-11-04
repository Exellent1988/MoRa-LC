#include "lvgl_display.h"
#include <Arduino.h>
#include "../core/lv_conf.h"

// Static instance for callbacks
LVGLDisplay* LVGLDisplay::_instance = nullptr;

LVGLDisplay::LVGLDisplay(Display* display)
    : _display(display)
    , _lvgl_display(nullptr)  // Will be set by lv_disp_drv_register()
    , _initialized(false) {
    _instance = this;
}

LVGLDisplay::~LVGLDisplay() {
    _instance = nullptr;
}

bool LVGLDisplay::begin() {
    if (!_display || !_display->isReady()) {
        Serial.println("[LVGL] ERROR: Display not initialized");
        return false;
    }
    
    Serial.println("[LVGL] Initializing LVGL8...");
    
    // Initialize LVGL
    lv_init();
    
    // Create display buffer (LVGL8 API)
    // Use double buffering for smoother updates
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[SCREEN_WIDTH * SCREEN_HEIGHT / 10];  // 10 lines buffer
    static lv_color_t buf2[SCREEN_WIDTH * SCREEN_HEIGHT / 10];
    
    // Initialize display buffer
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCREEN_WIDTH * SCREEN_HEIGHT / 10);
    
    // Create display descriptor
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = flushCallback;
    disp_drv.draw_buf = &draw_buf;
    _lvgl_display = lv_disp_drv_register(&disp_drv);
    
    // Create input device (touch)
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpadReadCallback;
    lv_indev_drv_register(&indev_drv);
    
    // LVGL tick will be called from update() based on millis()
    // No timer interrupt needed - simpler and more reliable
    
    _initialized = true;
    Serial.println("[LVGL] LVGL8 initialized successfully");
    return true;
}

void LVGLDisplay::update() {
    if (!_initialized) return;

    // With LV_TICK_CUSTOM, LVGL uses millis() directly, no need for lv_tick_inc()
    // Just call timer handler in loop
    // LVGL8 handles dirty rectangles automatically for optimal performance
    lv_timer_handler();
}

void LVGLDisplay::flushCallback(lv_disp_drv_t* disp_drv, const lv_area_t* area, lv_color_t* color_p) {
    if (!_instance || !_instance->_display) {
        lv_disp_flush_ready(disp_drv);
        return;
    }
    
    LGFX& lgfx = _instance->_display->getLGFX();
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    int32_t x2 = area->x2;
    int32_t y2 = area->y2;
    
    uint32_t w = (x2 - x1 + 1);
    uint32_t h = (y2 - y1 + 1);
    
    // LVGL8: color_p is already in RGB565 format (16-bit) when LV_COLOR_DEPTH=16
    // Use pushImage for efficient pixel transfer (similar to Cheap Yellow Display example)
    lgfx.startWrite();
    lgfx.pushImage(x1, y1, w, h, (uint16_t*)color_p);
    lgfx.endWrite();
    
    // Tell LVGL that flushing is done
    lv_disp_flush_ready(disp_drv);
}

void LVGLDisplay::touchpadReadCallback(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
    if (!_instance || !_instance->_display) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    
    uint16_t x, y;
    if (_instance->_display->getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

