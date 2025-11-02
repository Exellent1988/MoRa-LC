#ifndef TOUCH_WRAPPER_H
#define TOUCH_WRAPPER_H

#include <Arduino.h>

// Global Touch Wrapper
// Unterstützt sowohl GT911 (I2C) als auch XPT2046 (SPI)

extern bool getTouchCoordinates(uint16_t* x, uint16_t* y);

#endif // TOUCH_WRAPPER_H

