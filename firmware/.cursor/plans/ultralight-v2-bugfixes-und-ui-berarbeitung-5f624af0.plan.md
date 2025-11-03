<!-- 5f624af0-c04e-4464-8cb0-c488dfae5e60 2def4567-427d-431a-afc9-b62648f76104 -->
# Complete Firmware Rewrite - MoRa-LC UltraLight v3

## Goals

- **Performance**: Smooth 60fps UI, no flickering, optimized BLE scanning
- **Modern UI**: Icon-based design, unified theme, clean layout
- **Architecture**: Modular, maintainable, testable code structure
- **BLE Library**: Evaluate and potentially replace NimBLE if better alternatives exist
- **Keep**: LovyanGFX (working perfectly)

## Phase 1: Architecture & Foundation

### 1.1 New Project Structure

```
src/ultralight_v3/
├── core/
│   ├── system.h/cpp          # System initialization, watchdog
│   ├── task_manager.h/cpp    # FreeRTOS task management
│   └── config.h              # Centralized configuration
├── hardware/
│   ├── display.h/cpp         # LovyanGFX wrapper (minimal changes)
│   ├── touch.h/cpp           # Touch input handler
│   ├── sd_card.h/cpp         # SD card abstraction
│   └── ble.h/cpp             # BLE abstraction layer
├── services/
│   ├── beacon_service.h/cpp  # Beacon scanning & tracking
│   ├── lap_counter_service.h/cpp  # Lap counting logic
│   ├── data_logger_service.h/cpp  # SD card logging
│   └── persistence_service.h/cpp  # NVS storage
├── ui/
│   ├── theme.h               # Colors, fonts, icons, spacing
│   ├── widgets/
│   │   ├── button.h/cpp      # Icon buttons, text buttons
│   │   ├── list.h/cpp        # Scrollable lists
│   │   ├── dialog.h/cpp      # Modal dialogs
│   │   └── icons.h           # Icon definitions (bitmaps/sprites)
│   ├── screens/
│   │   ├── base_screen.h/cpp  # Base class for all screens
│   │   ├── home_screen.h/cpp
│   │   ├── teams_screen.h/cpp
│   │   ├── race_screen.h/cpp
│   │   └── ... (one file per screen)
│   └── navigation.h/cpp      # Screen navigation manager
└── main.cpp                   # Entry point, minimal code
```

### 1.2 Design System (`ui/theme.h`)

- **Color Palette**: Unified color scheme with semantic colors
- **Typography**: Consistent font sizes (icon fonts + text)
- **Spacing**: 8px grid system
- **Icons**: Bitmap icons (16x16, 24x24, 32x32) for common actions
  - Home, Settings, Back, Play, Pause, Stop
  - Team, Beacon, Race, Results
  - Up/Down arrows, Check, X, Edit
- **Components**: Reusable UI components with consistent styling

## Phase 2: BLE Library Evaluation & Implementation

### 2.1 Evaluate BLE Libraries

**Options to test:**

1. **NimBLE** (current) - Pros: Feature-rich, active. Cons: Logging issues, complex
2. **ESP32 BLE Arduino** - Pros: Simple, official. Cons: Less features, older
3. **ESP-IDF BLE APIs** - Pros: Native, performant. Cons: More complex, C-based

**Evaluation Criteria:**

- Performance (scan latency, callback speed)
- Memory usage
- Stability during long scans
- Ease of use
- Logging control

### 2.2 BLE Service Implementation

**New `beacon_service.h/cpp`:**

- Abstract BLE operations behind clean interface
- Handle scanning state management
- RSSI filtering and averaging
- Beacon presence tracking with hysteresis
- Thread-safe beacon data access
- Optimized for race mode (continuous scanning, no cleanup during race)

### 2.3 Performance Optimizations

- **Callback optimization**: Keep callbacks minimal, defer heavy work
- **Memory management**: Pre-allocate beacon storage, avoid String copies
- **Scan intervals**: Adaptive intervals (fast during race, slower during setup)
- **No cleanup during race**: Disable `clearOldBeacons` completely during race

## Phase 3: UI Framework & Components

### 3.1 Base Screen System

**`base_screen.h`:**

```cpp
class BaseScreen {
public:
    virtual void draw() = 0;
    virtual void update(uint32_t deltaTime) = 0;
    virtual bool handleTouch(uint16_t x, uint16_t y) = 0;
    virtual void onEnter() {}
    virtual void onExit() {}
protected:
    void drawHeader(const String& title, bool showBack);
    void drawFooter();
};
```

### 3.2 Widget System

**Icon Buttons:**

- Bitmap-based icons (no text where possible)
- Consistent sizing (40x40px standard)
- Hover/pressed states
- Touch feedback

**Lists:**

- Virtual scrolling (only render visible items)
- Smooth scroll animations
- Pull-to-refresh for beacon lists
- Optimized for 4+ teams

**Dialogs:**

- Non-blocking modal system
- Confirmation dialogs with icons
- Toast notifications (non-blocking messages)

### 3.3 Icon System

**Implementation Options:**

1. **Bitmap arrays** - Simple, fast, low memory
2. **LovyanGFX sprites** - Better scaling, more flexible
3. **Font-based icons** - Easy to use, but limited

**Recommendation**: Start with bitmap arrays (16x16, 24x24 RGB565), upgrade to sprites if needed

## Phase 4: Screen Redesign with Icons

### 4.1 Home Screen

- Large icon buttons (80x80px) instead of text buttons
- Icons: Race, Teams, Results, Settings
- Minimal text, clean layout

### 4.2 Teams Screen

- List with team icons/avatars
- Icon buttons for: Add, Edit, Delete
- Quick actions with icons

### 4.3 Race Setup Screen

- Icon-based duration controls (+5, -5, +15 icons)
- Visual team selection with checkmarks
- Large "START" icon button

### 4.4 Race Running Screen

- **Critical**: Optimized for performance
  - Only update changed values (time, laps)
  - Use dirty flags, not full redraws
  - Render to off-screen buffer, swap (if possible)
  - 60fps target (16ms per frame max)
- Large time display with icon
- Leaderboard with team icons
- Icon buttons for Pause/Stop (small, corners)

### 4.5 Beacon Assignment Screens

- Visual beacon indicators (signal strength icons)
- Distance visualization
- Non-blocking assignment (no overlay dialogs)
- Smooth list updates

## Phase 5: Performance Optimizations

### 5.1 Display Optimization

- **Dirty rectangle updates**: Only redraw changed areas
- **Frame buffering**: Use LovyanGFX frame buffer if available
- **Partial updates**: Update only time/laps during race
- **Double buffering**: If display supports it

### 5.2 Memory Management

- **Stack allocation**: Prefer stack over heap for small objects
- **String pooling**: Reuse String objects, avoid copies
- **Beacon storage**: Fixed-size array, not map (if possible)
- **UI state**: Minimize state variables, use bit flags

### 5.3 Task Management

- **FreeRTOS tasks**: Separate tasks for:
  - UI rendering (high priority, 16ms period)
  - BLE scanning (medium priority)
  - Data logging (low priority)
- **Inter-task communication**: Queues for beacon data

## Phase 6: Data Layer Improvements

### 6.1 Persistence Service

- Clean interface for NVS operations
- Type-safe storage/retrieval
- Migration support for config changes

### 6.2 Data Logger Service

- Async logging (queue-based)
- Batch writes to SD card
- Error recovery
- Automatic `/races` directory creation

### 6.3 Lap Counter Service

- Separated from UI logic
- Event-based notifications for lap events
- Thread-safe operations

## Phase 7: Testing & Validation

### 7.1 Unit Tests

- Service layer tests (mock hardware)
- UI component tests
- Algorithm tests (lap detection logic)

### 7.2 Integration Tests

- Full race simulation
- Beacon tracking accuracy
- Memory leak detection
- Performance profiling

### 7.3 Field Testing

- Real-world race scenarios
- Multiple beacon testing
- Long-duration stability
- Battery life testing

## Implementation Order

1. **Week 1**: Phase 1 (Architecture) + Phase 2.1 (BLE evaluation)
2. **Week 2**: Phase 2.2-2.3 (BLE implementation) + Phase 3 (UI framework)
3. **Week 3**: Phase 4 (Screen redesign)
4. **Week 4**: Phase 5 (Performance) + Phase 6 (Data layer)
5. **Week 5**: Phase 7 (Testing) + Bug fixes

## Migration Strategy

- Keep `ultralight_v2` as reference
- Build `ultralight_v3` in parallel
- Migrate features incrementally
- Test each screen before moving to next
- Maintain backward compatibility for data (NVS, SD)

## Key Decisions Needed

1. **UI Library**: Test LVGL9 vs Direct LovyanGFX - decision after Phase 3.1 evaluation

   - Priority: Evaluate LVGL9 performance first (likely better choice for icons/design)

2. **BLE Library**: Test NimBLE vs ESP32 BLE Arduino vs Ghost_ESP patterns - decision after Phase 2.1

   - Priority: Study Ghost_ESP implementation first, adopt best patterns

3. **Icon Format**: LVGL icons vs Bitmaps vs Sprites - depends on UI library choice
4. **Task System**: FreeRTOS tasks vs simple loop - recommend tasks for performance
5. **Update Strategy**: LVGL refresh vs Dirty rectangles - depends on UI library choice

## Success Criteria

- No visible flickering on race screen
- Smooth 60fps UI updates
- Beacon tracking accurate (no false "WEG" messages)
- All screens use icons where appropriate
- Unified design language throughout
- Code is modular and maintainable
- Memory usage < 80% of available
- Battery life acceptable for race duration