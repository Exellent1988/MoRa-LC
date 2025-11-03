#ifndef UI_HELPER_H
#define UI_HELPER_H

#include <Arduino.h>
#include "display_manager.h"
#include "config.h"

// Forward declarations
extern DisplayManager display;

// Helper Functions
void drawHeader(const String& title, bool showBack);
void drawButton(int x, int y, int w, int h, const String& text, uint16_t color);
bool isTouchInRect(uint16_t tx, uint16_t ty, int x, int y, int w, int h);
void showMessage(const String& title, const String& message, uint16_t color);

#endif // UI_HELPER_H

