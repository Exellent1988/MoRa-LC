#ifndef UI_HELPER_H
#define UI_HELPER_H

#include <Arduino.h>
#include "config.h"

// Forward declarations
extern class DisplayManager display;
extern struct UIState uiState;

// UI Helper Functions
bool isTouchInRect(uint16_t tx, uint16_t ty, int x, int y, int w, int h);
void drawButton(int x, int y, int w, int h, const String& text, uint16_t color = COLOR_BUTTON);
void drawHeader(const String& title, bool showBack = false);
void showMessage(const String& title, const String& message, uint16_t color);

// Keyboard input functions
String inputText(const String& prompt, const String& defaultValue = "");
uint32_t inputNumber(const String& prompt, uint32_t defaultValue, uint32_t min, uint32_t max);

// Beacon MAC address formatting (show only last 4 chars for c3:00:00:XX:XX:XX)
String formatBeaconMAC(const String& macAddress);

#endif // UI_HELPER_H
