#include "touch.h"
#include "display.h"

TouchHandler::TouchHandler()
    : _display(nullptr)
    , _debounceMs(TOUCH_DEBOUNCE)
    , _lastX(0)
    , _lastY(0)
    , _lastTouchTime(0)
    , _lastTouched(false) {
}

TouchHandler::~TouchHandler() {
}

void TouchHandler::begin(Display* display) {
    _display = display;
}

bool TouchHandler::isTouched() {
    if (!_display || !_display->isReady()) {
        return false;
    }
    return _display->isTouched();
}

bool TouchHandler::getTouch(uint16_t* x, uint16_t* y) {
    if (!_display || !_display->isReady()) {
        return false;
    }
    
    // Debounce check
    uint32_t now = millis();
    if (now - _lastTouchTime < _debounceMs && _lastTouched) {
        return false;
    }
    
    // Get touch from display
    if (_display->getTouch(x, y)) {
        _lastX = *x;
        _lastY = *y;
        _lastTouchTime = now;
        _lastTouched = true;
        return true;
    }
    
    _lastTouched = false;
    return false;
}

