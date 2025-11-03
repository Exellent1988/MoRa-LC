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

// Keyboard input functions
String inputText(const String& prompt, const String& defaultValue = "");
uint32_t inputNumber(const String& prompt, uint32_t defaultValue, uint32_t min, uint32_t max);

#endif // UI_HELPER_H

