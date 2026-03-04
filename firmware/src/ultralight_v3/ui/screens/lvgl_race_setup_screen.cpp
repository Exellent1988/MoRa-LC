#include "lvgl_race_setup_screen.h"
#include "lvgl_race_running_screen.h"
#include "../navigation_lvgl.h"
#include <Arduino.h>

LVGLRaceSetupScreen::LVGLRaceSetupScreen(LVGLDisplay* lvglDisplay)
    : LVGLBaseScreen(lvglDisplay)
    , _navigation(nullptr)
    , _raceRunningScreen(nullptr)
    , _controlsContainer(nullptr)
    , _nameTextArea(nullptr)
    , _durationValue(nullptr)
    , _btnStart(nullptr)
    , _keyboard(nullptr)
    , _raceDuration(60)
    , _keyboardVisible(false) {
}

LVGLRaceSetupScreen::~LVGLRaceSetupScreen() {
}

void LVGLRaceSetupScreen::onEnter() {
    if (!_lvglDisplay || !_lvglDisplay->isReady()) return;
    
    Serial.println("[LVGLRaceSetup] Creating race setup screen...");
    
    _screen = _lvglDisplay->getScreen();
    lv_obj_clean(_screen);
    _controlsContainer = nullptr;
    _nameTextArea = nullptr;
    _keyboard = nullptr;
    _keyboardVisible = false;
    
    lv_obj_set_style_bg_color(_screen, rgb565ToLVGL(Colors::BACKGROUND), LV_PART_MAIN);
    
    createHeader("Race Setup", true, backBtnEventHandler, this);
    
    // Controls container (everything below header, hidden when keyboard shows)
    int contentY = HEADER_HEIGHT + 2;
    int contentH = SCREEN_HEIGHT - contentY;
    _controlsContainer = lv_obj_create(_screen);
    lv_obj_set_size(_controlsContainer, SCREEN_WIDTH, contentH);
    lv_obj_set_pos(_controlsContainer, 0, contentY);
    lv_obj_set_style_bg_opa(_controlsContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_controlsContainer, 0, 0);
    lv_obj_set_style_pad_all(_controlsContainer, 0, 0);
    lv_obj_clear_flag(_controlsContainer, LV_OBJ_FLAG_SCROLLABLE);
    
    // Race name input
    int y = 4;
    lv_obj_t* nameLabel = lv_label_create(_controlsContainer);
    lv_label_set_text(nameLabel, "Race Name:");
    lv_obj_set_style_text_color(nameLabel, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_pos(nameLabel, Spacing::SM, y);
    y += 18;
    
    _nameTextArea = lv_textarea_create(_controlsContainer);
    lv_obj_set_size(_nameTextArea, SCREEN_WIDTH - 2 * Spacing::SM, 28);
    lv_obj_set_pos(_nameTextArea, Spacing::SM, y);
    lv_textarea_set_placeholder_text(_nameTextArea, "Race 1");
    lv_textarea_set_one_line(_nameTextArea, true);
    lv_obj_set_style_bg_color(_nameTextArea, rgb565ToLVGL(Colors::SURFACE), 0);
    lv_obj_set_style_text_color(_nameTextArea, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_style_border_width(_nameTextArea, 1, 0);
    lv_obj_set_style_border_color(_nameTextArea, rgb565ToLVGL(Colors::BORDER), 0);
    lv_obj_set_style_pad_ver(_nameTextArea, 4, 0);
    lv_obj_add_event_cb(_nameTextArea, nameTextAreaEventHandler, LV_EVENT_CLICKED, this);
    y += 36;
    
    // Duration label
    lv_obj_t* durLabel = lv_label_create(_controlsContainer);
    lv_label_set_text(durLabel, "Duration:");
    lv_obj_set_style_text_color(durLabel, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_pos(durLabel, Spacing::SM, y);
    y += 18;
    
    // Duration controls: [-5] [value] [+5] [+15]
    int btnW = 50;
    int btnH = 40;
    int valW = SCREEN_WIDTH - 4 * Spacing::SM - 3 * btnW;
    
    lv_obj_t* btnMinus = lv_btn_create(_controlsContainer);
    lv_obj_set_size(btnMinus, btnW, btnH);
    lv_obj_set_pos(btnMinus, Spacing::SM, y);
    lv_obj_set_style_bg_color(btnMinus, rgb565ToLVGL(Colors::BUTTON), 0);
    lv_obj_add_event_cb(btnMinus, durationMinusEventHandler, LV_EVENT_CLICKED, this);
    lv_obj_t* lblM = lv_label_create(btnMinus);
    lv_label_set_text(lblM, "-5");
    lv_obj_center(lblM);
    lv_obj_set_style_text_color(lblM, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    
    _durationValue = lv_label_create(_controlsContainer);
    lv_obj_set_size(_durationValue, valW, btnH);
    lv_obj_set_pos(_durationValue, Spacing::SM + btnW + Spacing::SM, y);
    lv_obj_set_style_text_color(_durationValue, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_style_text_align(_durationValue, LV_TEXT_ALIGN_CENTER, 0);
    extern const lv_font_t lv_font_montserrat_18;
    lv_obj_set_style_text_font(_durationValue, &lv_font_montserrat_18, 0);
    
    lv_obj_t* btnPlus = lv_btn_create(_controlsContainer);
    lv_obj_set_size(btnPlus, btnW, btnH);
    lv_obj_set_pos(btnPlus, SCREEN_WIDTH - 2 * Spacing::SM - 2 * btnW, y);
    lv_obj_set_style_bg_color(btnPlus, rgb565ToLVGL(Colors::BUTTON), 0);
    lv_obj_add_event_cb(btnPlus, durationPlusEventHandler, LV_EVENT_CLICKED, this);
    lv_obj_t* lblP = lv_label_create(btnPlus);
    lv_label_set_text(lblP, "+5");
    lv_obj_center(lblP);
    lv_obj_set_style_text_color(lblP, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    
    lv_obj_t* btnBigPlus = lv_btn_create(_controlsContainer);
    lv_obj_set_size(btnBigPlus, btnW, btnH);
    lv_obj_set_pos(btnBigPlus, SCREEN_WIDTH - Spacing::SM - btnW, y);
    lv_obj_set_style_bg_color(btnBigPlus, rgb565ToLVGL(Colors::BUTTON), 0);
    lv_obj_add_event_cb(btnBigPlus, durationBigPlusEventHandler, LV_EVENT_CLICKED, this);
    lv_obj_t* lblBP = lv_label_create(btnBigPlus);
    lv_label_set_text(lblBP, "+15");
    lv_obj_center(lblBP);
    lv_obj_set_style_text_color(lblBP, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    
    y += btnH + 12;
    
    // START button
    _btnStart = lv_btn_create(_controlsContainer);
    lv_obj_set_size(_btnStart, SCREEN_WIDTH - 2 * Spacing::SM, 42);
    lv_obj_set_pos(_btnStart, Spacing::SM, y);
    lv_obj_set_style_bg_color(_btnStart, rgb565ToLVGL(Colors::SECONDARY), 0);
    lv_obj_add_event_cb(_btnStart, startBtnEventHandler, LV_EVENT_CLICKED, this);
    lv_obj_t* startLabel = lv_label_create(_btnStart);
    lv_label_set_text(startLabel, "START");
    lv_obj_center(startLabel);
    lv_obj_set_style_text_color(startLabel, rgb565ToLVGL(Colors::BUTTON_TEXT), 0);
    lv_obj_set_style_text_font(startLabel, &lv_font_montserrat_18, 0);
    
    updateDurationDisplay();
    
    Serial.println("[LVGLRaceSetup] Race setup screen created");
}

void LVGLRaceSetupScreen::onExit() {
    hideKeyboard();
}

const char* LVGLRaceSetupScreen::getRaceName() const {
    if (_nameTextArea) {
        const char* text = lv_textarea_get_text(_nameTextArea);
        if (text && strlen(text) > 0) return text;
    }
    return nullptr;
}

void LVGLRaceSetupScreen::showKeyboard() {
    if (_keyboardVisible) return;
    _keyboardVisible = true;
    
    lv_obj_add_flag(_controlsContainer, LV_OBJ_FLAG_HIDDEN);
    
    // Recreate text area on screen (outside container)
    int y = HEADER_HEIGHT + 2;
    _nameTextArea = lv_textarea_create(_screen);
    lv_obj_set_size(_nameTextArea, SCREEN_WIDTH - 2 * Spacing::SM, 28);
    lv_obj_set_pos(_nameTextArea, Spacing::SM, y);
    lv_textarea_set_placeholder_text(_nameTextArea, "Race 1");
    lv_textarea_set_one_line(_nameTextArea, true);
    lv_obj_set_style_bg_color(_nameTextArea, rgb565ToLVGL(Colors::SURFACE), 0);
    lv_obj_set_style_text_color(_nameTextArea, rgb565ToLVGL(Colors::TEXT), 0);
    lv_obj_set_style_border_width(_nameTextArea, 1, 0);
    lv_obj_set_style_border_color(_nameTextArea, rgb565ToLVGL(Colors::BORDER), 0);
    lv_obj_set_style_pad_ver(_nameTextArea, 4, 0);
    
    y += 30;
    
    _keyboard = lv_keyboard_create(_screen);
    lv_keyboard_set_textarea(_keyboard, _nameTextArea);
    lv_keyboard_set_mode(_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_set_size(_keyboard, SCREEN_WIDTH, SCREEN_HEIGHT - y);
    lv_obj_align(_keyboard, LV_ALIGN_TOP_LEFT, 0, y);
    lv_obj_set_style_bg_color(_keyboard, rgb565ToLVGL(Colors::BACKGROUND), 0);
    lv_obj_set_style_pad_all(_keyboard, 2, 0);
    lv_obj_set_style_pad_gap(_keyboard, 2, 0);
    lv_obj_add_event_cb(_keyboard, keyboardEventHandler, LV_EVENT_READY, this);
    lv_obj_add_event_cb(_keyboard, keyboardEventHandler, LV_EVENT_CANCEL, this);
}

void LVGLRaceSetupScreen::hideKeyboard() {
    if (!_keyboardVisible) return;
    _keyboardVisible = false;
    
    // Get the typed text before destroying
    String typedName = "";
    if (_nameTextArea) {
        const char* text = lv_textarea_get_text(_nameTextArea);
        if (text) typedName = String(text);
        lv_obj_del(_nameTextArea);
        _nameTextArea = nullptr;
    }
    
    if (_keyboard) {
        lv_obj_del(_keyboard);
        _keyboard = nullptr;
    }
    
    // Show controls and restore text to the container's textarea
    lv_obj_clear_flag(_controlsContainer, LV_OBJ_FLAG_HIDDEN);
    
    // Find the textarea in the controls container (first textarea child)
    uint32_t cnt = lv_obj_get_child_cnt(_controlsContainer);
    for (uint32_t i = 0; i < cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(_controlsContainer, i);
        if (lv_obj_check_type(child, &lv_textarea_class)) {
            _nameTextArea = child;
            if (typedName.length() > 0) {
                lv_textarea_set_text(_nameTextArea, typedName.c_str());
            }
            break;
        }
    }
}

void LVGLRaceSetupScreen::updateDurationDisplay() {
    if (!_durationValue) return;
    char buf[16];
    snprintf(buf, sizeof(buf), "%lu Min", _raceDuration);
    lv_label_set_text(_durationValue, buf);
}

void LVGLRaceSetupScreen::backBtnEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (!screen) return;
    if (screen->_keyboardVisible) {
        screen->hideKeyboard();
        return;
    }
    if (screen->_navigation) {
        Serial.println("[LVGLRaceSetup] Back button clicked");
        screen->_navigation->goBack();
    }
}

void LVGLRaceSetupScreen::nameTextAreaEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (screen) {
        screen->showKeyboard();
    }
}

void LVGLRaceSetupScreen::keyboardEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (screen) {
        screen->hideKeyboard();
    }
}

void LVGLRaceSetupScreen::durationMinusEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (!screen) return;
    if (screen->_raceDuration > 5) {
        screen->_raceDuration -= 5;
        screen->updateDurationDisplay();
    }
}

void LVGLRaceSetupScreen::durationPlusEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (!screen) return;
    if (screen->_raceDuration < 180) {
        screen->_raceDuration += 5;
        screen->updateDurationDisplay();
    }
}

void LVGLRaceSetupScreen::durationBigPlusEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (!screen) return;
    if (screen->_raceDuration < 180) {
        screen->_raceDuration = min((uint32_t)180, screen->_raceDuration + 15);
        screen->updateDurationDisplay();
    }
}

void LVGLRaceSetupScreen::startBtnEventHandler(lv_event_t* e) {
    LVGLRaceSetupScreen* screen = (LVGLRaceSetupScreen*)lv_event_get_user_data(e);
    if (!screen || !screen->_navigation || !screen->_raceRunningScreen) return;
    
    const char* raceName = screen->getRaceName();
    Serial.printf("[LVGLRaceSetup] Start - Duration: %lu min, Name: %s\n", 
                  screen->_raceDuration, raceName ? raceName : "(auto)");
    
    screen->_raceRunningScreen->startRace(screen->_raceDuration, raceName);
    delay(10);
    screen->_navigation->setScreen(screen->_raceRunningScreen);
}
