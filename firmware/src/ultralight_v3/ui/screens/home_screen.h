#ifndef HOME_SCREEN_H
#define HOME_SCREEN_H

#include "base_screen.h"

/**
 * Home Screen
 * Main navigation screen with large icon buttons
 */
class HomeScreen : public BaseScreen {
public:
    HomeScreen();
    virtual ~HomeScreen();
    
    void draw() override;
    void update(uint32_t deltaTime) override {}
    bool handleTouch(uint16_t x, uint16_t y) override;
    
private:
    void drawIconButton(int x, int y, int w, int h, const String& label, IconID icon, uint16_t color);
};

#endif // HOME_SCREEN_H

