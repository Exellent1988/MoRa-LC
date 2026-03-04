---
name: Fix ultralight_v3 Critical Issues
overview: ""
todos:
  - id: 3f321473-b388-4a59-872f-231825974714
    content: Dialog show() Aufrufe in lvgl_settings_screen.cpp korrigieren (7 Zeilen)
    status: completed
  - id: ed7a4b5f-daf0-48bd-a66e-cfcf2ded8324
    content: Dialog delete im Settings Screen Destruktor hinzufügen
    status: completed
  - id: 5dba2592-8281-4693-844a-093bf0b68db6
    content: Navigation Stack mit Duplikat-Check und Limit versehen
    status: completed
  - id: 164dfd1d-e563-4318-ba0a-2549a9cb9e2a
    content: Dialog-Manager Singleton implementieren (Header + Implementation)
    status: completed
  - id: 5f0bab5a-25fa-4de8-88a4-c55089cac794
    content: Dialog-Manager in Settings Screen und main.cpp integrieren
    status: completed
  - id: 375d27d6-0a07-4a4e-8c44-57bf6e0a0ff3
    content: config.h Include in lvgl_settings_screen.cpp hinzufügen
    status: completed
  - id: 9ac4adc8-78dc-411e-95f9-b3d4cad8366c
    content: Kompiliertest durchführen und Fehler prüfen
    status: completed
isProject: false
---

# Fix ultralight_v3 Critical Issues

## Übersicht

Behebung kritischer Fehler und Implementierung eines sauberen Dialog-Systems:

- Kompilierfehler bei LVGLDialog::show() Aufrufen
- Memory Leaks in Dialog-Verwaltung
- Navigation Stack Overflow-Prevention
- Globaler Dialog-Manager als Singleton

## Kritische Fixes

### 1. Kompilierfehler in lvgl_settings_screen.cpp beheben

**Problem**: show() wird mit 7 statt 5 Parametern aufgerufen

**Files**: `firmware/src/ultralight_v3/ui/screens/lvgl_settings_screen.cpp`

Alle Dialog-Aufrufe anpassen (7 Zeilen):

- Zeile 116, 126, 134, 143, 148, 159, 179
- Von: `show(title, msg, "OK", nullptr, nullptr, nullptr, nullptr)`
- Zu: `show(title, msg, "OK", nullptr, nullptr)`

### 2. Memory Leak in LVGLSettingsScreen fixen

**Problem**: Dialog wird mit `new` erstellt, aber nie gelöscht

**Files**: `firmware/src/ultralight_v3/ui/screens/lvgl_settings_screen.cpp`

Im Destruktor (Zeile 19) hinzufügen:

```cpp
if (_dialog) {
    delete _dialog;
    _dialog = nullptr;
}
```

### 3. Navigation Stack Overflow Prevention

**Problem**: Stack wächst bei wiederholter Forward-Navigation ohne Limit

**Files**: `firmware/src/ultralight_v3/ui/navigation_lvgl.cpp`, `navigation_lvgl.h`

Änderungen:

- Duplikat-Check: Wenn Screen bereits im Stack, nicht erneut pushen
- Stack-Limit: Max 10 Screens im Stack (sollte für normale Navigation reichen)
- Logging verbessern für besseres Debugging

### 4. Globaler Dialog-Manager implementieren

**Problem**: Jeder Screen erstellt eigenen Dialog, führt zu Memory-Overhead

**Neue Files**:

- `firmware/src/ultralight_v3/ui/widgets/dialog_manager.h`
- `firmware/src/ultralight_v3/ui/widgets/dialog_manager.cpp`

**Geänderte Files**:

- `lvgl_settings_screen.h/.cpp` - Dialog-Member entfernen, Manager nutzen
- `main.cpp` - Dialog-Manager initialisieren

**Design**:

- Singleton Pattern für globalen Zugriff
- Nutzt vorhandene LVGLDialog Klasse
- API: `DialogManager::getInstance()->show(...)`
- Wird in main.cpp initialisiert mit Root-Screen

### 5. Config Header in lvgl_settings_screen.cpp hinzufügen

**Problem**: BLE_* Konstanten werden genutzt aber config.h nicht inkludiert

**Files**: `firmware/src/ultralight_v3/ui/screens/lvgl_settings_screen.cpp`

Zeile 1 hinzufügen: `#include "../../core/config.h"`

## Testing

Nach den Änderungen:

- Kompiliertest: `pio run -e ultralight_v3`
- Prüfen ob alle Fehler behoben sind
- Keine neuen Linter-Fehler

## Commit

```
fix(ultralight_v3): kritische UI Fehler behoben

- Kompilierfehler bei Dialog::show() Aufrufen korrigiert
- Memory Leak in Settings Screen behoben
- Navigation Stack mit Overflow-Schutz versehen
- Globaler Dialog-Manager als Singleton implementiert
- Fehlende config.h Include hinzugefügt
```