# UI Flow Documentation - MoRa-LC UltraLight v2

## Screen Overview

### Screen States (Enum)
```cpp
enum Screen {
    SCREEN_HOME,                  // 0 - Hauptmenü
    SCREEN_TEAMS,                 // 1 - Team-Verwaltung
    SCREEN_TEAM_EDIT,             // 2 - Team bearbeiten
    SCREEN_TEAM_BEACON_ASSIGN,    // 3 - Beacon zuordnen (Nearest)
    SCREEN_BEACON_LIST,           // 4 - Beacon-Liste (alle sichtbaren)
    SCREEN_RACE_SETUP,            // 5 - Rennen einrichten
    SCREEN_RACE_RUNNING,          // 6 - Rennen läuft
    SCREEN_RACE_PAUSED,           // 7 - Rennen pausiert
    SCREEN_RACE_RESULTS,          // 8 - Rennen-Ergebnisse
    SCREEN_SETTINGS               // 9 - Einstellungen
};
```

## Navigation Flows

### 1. Main Menu (HOME)
**From:** Startup, Back from any top-level screen  
**To:**
- `Neues Rennen` → **RACE_SETUP**
- `Teams` → **TEAMS**
- `Ergebnisse` → **RACE_RESULTS** (historisch)
- `Einstellungen` → **SETTINGS**

**Back:** N/A (Root screen)

---

### 2. Team Management Flow

#### 2.1 TEAMS (Team-Liste)
**From:** HOME  
**To:**
- Team klicken → **TEAM_EDIT** (existing team)
- `+ Hinzufügen` → **TEAM_EDIT** (new team)

**Back:** → **HOME**

#### 2.2 TEAM_EDIT (Team bearbeiten)
**From:** TEAMS  
**To:**
- `Name ändern` → Keyboard Input (in-place)
- `Beacon zuordnen` → **TEAM_BEACON_ASSIGN** ⚠️ `previousScreen = TEAM_EDIT`
- `Speichern` → **TEAMS** (with save)
- `Löschen` → **TEAMS** (with delete)

**Back:** → **TEAMS** ❌ **BUG: Should use changeScreen(), not manual previousScreen**

#### 2.3 TEAM_BEACON_ASSIGN (Nearest Beacon)
**From:** TEAM_EDIT  
**To:**
- `Zuordnen` → **TEAM_EDIT** (assign + save)
- `Liste` → **BEACON_LIST**
- Bottom `Beacon-Liste anzeigen` → **BEACON_LIST**

**Back:** → `previousScreen` (should be TEAM_EDIT) ✅

**Special:** BLE scan auto-starts when entering this screen

#### 2.4 BEACON_LIST (Alle Beacons)
**From:** TEAM_BEACON_ASSIGN  
**To:**
- Beacon klicken (dist < 1m) → **TEAM_EDIT** (assign + save)
- Beacon klicken (dist >= 1m) → Warnung + stay

**Back:** → `previousScreen` (should be TEAM_BEACON_ASSIGN) ✅

**Special:** BLE scan continues from previous screen

---

### 3. Race Flow

#### 3.1 RACE_SETUP (Rennen einrichten)
**From:** HOME  
**To:**
- `Start` → **RACE_RUNNING** (with raceRunning = true)

**Back:** → **HOME**

**Fields:**
- Race Name (Keyboard Input)
- Duration (+/- buttons, +5/+15 min)
- Team Selection (checkboxes)

#### 3.2 RACE_RUNNING (Rennen läuft)
**From:** RACE_SETUP, RACE_PAUSED (Fortsetzen)  
**To:**
- `Pause` → **RACE_PAUSED** (raceRunning = false)
- `Beenden` → **RACE_RESULTS** (raceRunning = false) ⚠️ `previousScreen = RACE_RUNNING`

**Back:** N/A (no back button during race)

**Display:**
- Elapsed Time
- Team Standings (Live)
- Lap Counts

#### 3.3 RACE_PAUSED (Rennen pausiert)
**From:** RACE_RUNNING (Pause)  
**To:**
- `Fortsetzen` → **RACE_RUNNING** (raceRunning = true)
- `Beenden` → **RACE_RESULTS** (raceRunning = false) ⚠️ `previousScreen = RACE_PAUSED`

**Back:** N/A (no back button)

**Display:** Same as RACE_RUNNING but frozen

#### 3.4 RACE_RESULTS (Ergebnisse)
**From:**
- HOME → `Ergebnisse` (historical view) ⚠️ `previousScreen = HOME`
- RACE_RUNNING → `Beenden` (current race) ⚠️ `previousScreen = RACE_RUNNING`
- RACE_PAUSED → `Beenden` (current race) ⚠️ `previousScreen = RACE_PAUSED`

**To:**
- Page navigation (historical races)

**Back:** → **HOME** ✅ **FIXED!**
- Always returns to HOME (race is ended when viewing results)
- Explicit handler prevents returning to RACE_RUNNING/PAUSED

**Display:**
- Final Standings
- Race Statistics
- Historical race selector

---

### 4. Settings Flow

#### 4.1 SETTINGS
**From:** HOME  
**To:**
- `Formatieren` → SD Format (in-place, with confirmation)
- `Kalibrieren` → Touch Calibration (in-place)
- RSSI +/- buttons (in-place, save to NVS)

**Back:** → **HOME**

---

## Known Issues & Bugs

### Critical Bugs

#### ✅ Fixed: RACE_RESULTS Back Button
**Problem:** Global back button handler was intercepting back button and using `previousScreen`, which returned to RACE_RUNNING/PAUSED instead of HOME  
**Location:** `handleTouch()` Line 666-677  
**Root Cause:** Global handler ran BEFORE switch-case, with `return` preventing explicit handler from being called  
**Fix:** Exclude RACE_RESULTS (and TEAMS) from global back button handler:
```cpp
if (uiState.currentScreen != SCREEN_HOME && 
    uiState.currentScreen != SCREEN_TEAMS && 
    uiState.currentScreen != SCREEN_RACE_RESULTS) {
    // Global back button handler
}
```
**Reasoning:** After a race ends (RUNNING/PAUSED → RESULTS), `previousScreen` = RACE_RUNNING/PAUSED.  
Using the generic handler would incorrectly return to the race screen.  
Explicit handler ensures RESULTS → HOME (correct after race ends).

#### 🐛 Bug #2: TEAM_EDIT Back Button (Commented Out)
**Problem:** Dead code - explicit back button handler was removed  
**Location:** `handleTeamEditTouch()` Line 795-798 (commented out)  
**Status:** Fixed - global handler now manages back button ✅

#### 🐛 Bug #3: Missing Touch Handlers in Switch
**Problem:** BEACON_ASSIGN, BEACON_LIST, RACE_* screens had no touch handlers  
**Location:** `handleTouch()` Line 677-710  
**Status:** Fixed - all handlers added ✅

### Minor Issues

#### ⚠️ Issue #1: previousScreen Management
**Problem:** TEAM_EDIT manually sets `previousScreen = SCREEN_TEAM_EDIT` before going to BEACON_ASSIGN  
**Location:** Line 832  
**Why:** To ensure BEACON screens can return to TEAM_EDIT  
**Status:** Works correctly, but inconsistent with auto-setting in `changeScreen()`

#### ⚠️ Issue #2: Race Results Context
**Problem:** No way to distinguish "historical results" from "current race results"  
**Suggestion:** Add a `bool fromActiveRace` flag to UIState  
**Impact:** Low - current behavior (always → HOME) is acceptable after race ends

---

## Screen State Diagram

```
                         ┌─────────┐
                         │  HOME   │
                         └────┬────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
        ▼                     ▼                     ▼
   ┌────────┐          ┌───────────┐         ┌──────────┐
   │ TEAMS  │          │RACE_SETUP │         │ SETTINGS │
   └───┬────┘          └─────┬─────┘         └──────────┘
       │                     │                      │
       │ ┌───────────────────┘                      │
       │ │                                          │
       ▼ ▼                                          │
  ┌────────────┐                                    │
  │ TEAM_EDIT  │                                    │
  └──────┬─────┘                                    │
         │                                          │
         ▼                                          │
  ┌──────────────────┐                              │
  │TEAM_BEACON_ASSIGN│◄────┐                        │
  └────────┬─────────┘     │                        │
           │               │                        │
           ▼               │                        │
      ┌────────────┐       │                        │
      │BEACON_LIST │───────┘                        │
      └────────────┘                                │
                                                    │
                                                    │
    ┌───────────────────────────────────────────────┘
    │
    ▼
┌─────────────┐     ┌──────────────┐     ┌──────────────┐
│RACE_RUNNING │◄───►│ RACE_PAUSED  │────►│RACE_RESULTS  │
└─────────────┘     └──────────────┘     └──────┬───────┘
       │                    │                    │
       └────────────────────┴────────────────────┘
                            │
                            ▼
                        ┌───────┐
                        │ HOME  │
                        └───────┘
```

---

## Back Button Behavior Summary

| Screen                | Back Button Destination | Status |
|-----------------------|-------------------------|--------|
| HOME                  | None (root)             | ✅     |
| TEAMS                 | HOME                    | ✅     |
| TEAM_EDIT             | TEAMS                   | ✅     |
| TEAM_BEACON_ASSIGN    | previousScreen (TEAM_EDIT) | ✅  |
| BEACON_LIST           | previousScreen (BEACON_ASSIGN) | ✅ |
| RACE_SETUP            | HOME                    | ✅     |
| RACE_RUNNING          | None (in race)          | ✅     |
| RACE_PAUSED           | None (in race)          | ✅     |
| RACE_RESULTS          | HOME                    | ✅     |
| SETTINGS              | HOME                    | ✅     |

**Legend:**
- ✅ Correct behavior
- ⚠️ Works, but could be improved
- ❌ Bug (fixed)

---

## Global Back Button Handler

Location: `handleTouch()` Lines 665-674

```cpp
if (uiState.currentScreen != SCREEN_HOME) {
    if (isTouchInRect(x, y, 5, 5, 40, 30)) {
        // Stop BLE scanning if on beacon assignment screens
        if (uiState.currentScreen == SCREEN_TEAM_BEACON_ASSIGN || 
            uiState.currentScreen == SCREEN_BEACON_LIST) {
            bleScanner.stopScan();
        }
        uiState.changeScreen(uiState.previousScreen);
        return;
    }
}
```

**Override Screens (with explicit back button handlers):**
- TEAMS (Line 754-756) → HOME
- RACE_RESULTS (Line 1180-1183) → HOME

**Why Override?** These screens need to always return to HOME, not `previousScreen`.

---

## Auto-BLE Scanning

BLE scanning auto-starts/stops based on screen:

**Auto-Start:**
- SCREEN_TEAM_BEACON_ASSIGN
- SCREEN_BEACON_LIST
- SCREEN_RACE_RUNNING (when `raceRunning == true`)

**Auto-Stop:**
- When leaving BEACON screens to any other screen (except BEACON_LIST ↔ BEACON_ASSIGN)
- When race stops (`raceRunning = false`)

---

## Recommendations

1. **✅ FIXED:** Add all missing touch handlers to switch-case
2. **✅ FIXED:** Remove dead code in TEAM_EDIT back button handler
3. **Consider:** Add `fromActiveRace` flag for better RESULTS context
4. **Consider:** Unify `previousScreen` management (either all manual or all auto)
5. **✅ ACCEPTED:** RACE_RESULTS → HOME is correct (race always ends when viewing results)

---

**Document Version:** 1.1  
**Last Updated:** 2025-11-03  
**Status:** All critical bugs fixed ✅  
**Changes v1.1:** Fixed RACE_RESULTS back button - excluded from global handler

