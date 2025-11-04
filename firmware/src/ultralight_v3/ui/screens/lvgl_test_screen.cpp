#include "lvgl_test_screen.h"
#include <Arduino.h>

LVGLTestScreen::LVGLTestScreen(LVGLDisplay* lvglDisplay)
    : _lvglDisplay(lvglDisplay)
    , _screen(nullptr)
    , _labelTitle(nullptr)
    , _labelStatus(nullptr)
    , _btnTest(nullptr) {
}

LVGLTestScreen::~LVGLTestScreen() {
    // LVGL objects are cleaned up automatically when screen is deleted
}

void LVGLTestScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) {
        Serial.println("[LVGLTestScreen] ERROR: LVGLDisplay not ready");
        return;
    }
    
    Serial.println("[LVGLTestScreen] Creating LVGL test screen...");
    
    // Get current screen
    _screen = _lvglDisplay->getScreen();
    
    // Clear screen
    lv_obj_clean(_screen);
    
    // Set background color to white for better visibility
    lv_obj_set_style_bg_color(_screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    // Create title label
    _labelTitle = lv_label_create(_screen);
    lv_label_set_text(_labelTitle, "LVGL Test Screen");
    lv_obj_set_style_text_color(_labelTitle, lv_color_hex(0x000000), 0);  // Black text on white background
    // Use default font (LVGL8 uses LV_FONT_DEFAULT or NULL for default)
    lv_obj_align(_labelTitle, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create status label
    _labelStatus = lv_label_create(_screen);
    lv_label_set_text(_labelStatus, "LVGL8 initialized!");
    lv_obj_set_style_text_color(_labelStatus, lv_color_hex(0x008000), 0);  // Dark green text on white background
    // Use default font
    lv_obj_align(_labelStatus, LV_ALIGN_TOP_MID, 0, 60);
    
    // Create test button
    _btnTest = lv_btn_create(_screen);
    lv_obj_set_size(_btnTest, 120, 50);
    lv_obj_align(_btnTest, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(_btnTest, btnEventHandler, LV_EVENT_CLICKED, this);
    
    // Button label
    lv_obj_t* btnLabel = lv_label_create(_btnTest);
    lv_label_set_text(btnLabel, "Click Me!");
    lv_obj_center(btnLabel);
    
    Serial.println("[LVGLTestScreen] Test screen created");
}

void LVGLTestScreen::update() {
    // LVGL updates are handled by LVGLDisplay::update() in main loop
    // This method is kept for future use (e.g., updating status)
}

void LVGLTestScreen::btnEventHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        LVGLTestScreen* screen = (LVGLTestScreen*)lv_event_get_user_data(e);
        if (screen && screen->_labelStatus) {
            static int clickCount = 0;
            clickCount++;
            char buf[32];
            snprintf(buf, sizeof(buf), "Clicked %d times!", clickCount);
            lv_label_set_text(screen->_labelStatus, buf);
            Serial.printf("[LVGLTestScreen] Button clicked %d times\n", clickCount);
        }
    }
}

