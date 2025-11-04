<!-- 7fdb7541-5f31-43d8-905d-3ba37f231a29 cccabef8-b5f3-4d52-868e-ae9abd99563a -->
# UltraLight v3 Firmware - Fertigstellung & Code-Review

## Aktueller Stand

Die ultralight_v3 Firmware ist ein kompletter Rewrite mit Clean Architecture. Die Struktur ist vollständig vorhanden:

**Vollständig implementiert:**

- Core System (SystemManager, TaskManager, MemoryManager)
- Hardware-Abstraktionen (Display/LVGL, Touch, SD Card, BLE)
- Services (BeaconService, LapCounterService, PersistenceService, DataLoggerService)
- UI Screens (Home, Teams, Race Setup, Race Running, Results, Settings, BLE Test)
- Navigation System (LVGL-basiert)
- BLE Integration (NimBLE, ESP32, ESP-IDF optional)

**Funktional:**

- BLE Beacon Scanning
- Lap Detection (RSSI-basiert)
- Team Management mit Persistence
- Race Control (Start/Stop/Pause)
- Leaderboard
- Timer-Anzeige

## Fehlende/Unvollständige Teile

### 1. SD Card Integration im DataLogger

- **Status:** TODO vorhanden, Datei-Erstellung nicht implementiert
- **Datei:** `firmware/src/ultralight_v3/services/data_logger_service.cpp:46`
- **Benötigt:** SDCard Interface verwenden, Dateien erstellen/schreiben

### 2. UI-Verbindungen

- **Settings Screen:** BLE Settings Dialog fehlt
- **Settings Screen:** Teams-Speicherung fehlt
- **Race Running Screen:** Navigation zu Results Screen beim Stop
- **Teams Screen:** LVGL Dialog für Bestätigungen

### 3. UI Font-Probleme

- **Problem:** Fonts werden inkonsistent verwendet
- **Details:** 
  - Labels verwenden manchmal keinen expliziten Font (nur Default)
  - Font-Checks mit `#ifdef` sind fehleranfällig
  - Umlaute werden vermieden (Font-Unterstützung unklar)
  - Font-Größen könnten zu klein/zu groß sein
- **Dateien:** `lvgl_base_screen.cpp`, alle Screen-Implementierungen
- **Benötigt:** Konsistente Font-Verwendung, Font-Verfügbarkeit prüfen, Lesbarkeit testen

### 4. UI Farb-Probleme

- **Problem:** Farben könnten auf Display schlecht lesbar sein
- **Details:**
  - BACKGROUND = 0xCE59 (Light Gray) könnte zu hell sein
  - Kontraste zwischen Text und Hintergrund prüfen
  - RGB565-Farbwerte auf tatsächliche Display-Darstellung testen
- **Dateien:** `theme.h`, alle Screen-Implementierungen
- **Benötigt:** Farb-Kontrast-Tests, ggf. Farb-Palette anpassen

### 5. BLE Scan-Verifikation

- **Status:** Implementierung vorhanden, aber nicht getestet
- **Mögliche Probleme:**
  - Scan-Parameter (Interval/Window) optimal?
  - Duplicate Filter = false korrekt für Race Mode?
  - RSSI-Threshold korrekt?
  - Callback-Performance ausreichend?
- **Dateien:** `ble_nimble.cpp`, `beacon_service.cpp`
- **Benötigt:** Code-Review, Logging-Analyse, Test-Szenarien

### 6. Icon-System

- **Status:** Platzhalter vorhanden
- **Datei:** `firmware/src/ultralight_v3/ui/widgets/icons.cpp`
- **Benötigt:** Eigene Icon-Bitmaps oder Font-basierte Icons

### 7. Race Results Integration

- Race Results Screen mit Daten aus LapCounter füllen
- Navigation von Race Running Screen testen

### 8. Code-Review & Testing

- Kompilierung auf Fehler prüfen
- Serial-Logs auf Warnungen/Fehler prüfen
- UI-Flows komplett testen
- BLE Scan-Performance testen
- Memory-Leaks prüfen

### 9. Dokumentation

- **Status:** STATUS.md ist veraltet (zeigt ultralight_v2 als 100%)
- **Benötigt:** 
  - STATUS.md aktualisieren (ultralight_v3 Stand)
  - README.md aktualisieren
  - Bekannte Probleme dokumentieren
  - Build-Anweisungen prüfen

## Implementierungsplan

### Phase 1: Code-Review & Problemanalyse

1. Kompilierung prüfen (alle Warnings/Errors)
2. Font-Verwendung analysieren und konsistent machen
3. Farb-Kontraste prüfen und ggf. anpassen
4. BLE Scan-Code durchgehen (Parameter, Callbacks, Performance)
5. Serial-Logs auf Probleme prüfen

### Phase 2: UI-Fixes

1. Font-System konsistent implementieren (alle Screens)
2. Farb-Palette testen und optimieren
3. Umlaute-Problem lösen (Font-Unterstützung oder Ersatz)
4. Lesbarkeit auf allen Screens prüfen

### Phase 3: SD Card Integration (Kritisch)

1. SDCard Interface in DataLoggerService integrieren
2. Datei-Erstellung in `/races/` Verzeichnis
3. CSV-Header schreiben
4. Log-Entries schreiben (flushQueue)
5. Race-End-Logging implementieren

### Phase 4: UI-Verbindungen vervollständigen

1. Settings Screen: BLE Settings Dialog/Screen
2. Settings Screen: Teams-Speicherung über PersistenceService
3. Race Running Screen: Navigation zu Results beim Stop
4. Teams Screen: LVGL Dialog für Lösch-Bestätigungen

### Phase 5: Race Results Screen

1. Results Screen mit Daten aus LapCounter füllen
2. Team-Liste mit Medaillen anzeigen
3. Beste Rundenzeiten anzeigen
4. Navigation testen

### Phase 6: BLE Scan-Verifikation

1. Scan-Parameter testen und optimieren
2. Callback-Performance messen
3. Race Mode Verhalten testen
4. RSSI-Threshold testen

### Phase 7: Testing & Dokumentation

1. Komplette UI-Flows testen
2. Memory-Leaks prüfen
3. STATUS.md aktualisieren
4. README.md aktualisieren
5. Bekannte Probleme dokumentieren

## Technische Details

### Font-Problem

- LVGL Montserrat-Fonts sind aktiviert (12, 14, 16, 18, 20, 22, 24)
- Default Font ist montserrat_14
- Problem: Fonts werden inkonsistent gesetzt (manchmal nur Default)
- Lösung: Konsistente Font-Verwendung in allen Screens

### Farb-Problem

- RGB565-Farben in theme.h definiert
- BACKGROUND = 0xCE59 (Light Gray) könnte Kontrast-Problem verursachen
- Lösung: Farb-Kontrast prüfen, ggf. dunklere Hintergrundfarbe

### BLE Scan

- NimBLE mit Duplicate Filter = false (für Updates)
- Scan Interval/Window: 100/99ms
- Race Mode: Kein Cleanup während Rennen
- Zu prüfen: Performance, Parameter, Callback-Timing

### SD Card Integration

```cpp
// In DataLoggerService::createRaceFile()
// Verwende: _sdCard->createFile() oder SdFat direkt
// Verzeichnis erstellen: /races/ falls nicht vorhanden
// Dateiname: race_<timestamp>.csv
```

## Prioritäten

**Hoch (für funktionierendes System):**

1. Code-Review & Problemanalyse
2. SD Card Integration (Daten-Logging)
3. UI Font/Farb-Fixes (Lesbarkeit)
4. Race Results Screen (Nach Rennen)
5. Navigation Race Running → Results

**Mittel (für vollständige Funktionalität):**

6. BLE Scan-Verifikation
7. Settings: Teams speichern
8. Settings: BLE Settings
9. Dialog-System

**Niedrig (kosmetisch):**

10. Icon-System
11. Polishing/UI-Verbesserungen

## Nächste Schritte

1. Code-Review durchführen (Kompilierung, Fonts, Farben, BLE)
2. UI Font/Farb-Probleme beheben
3. SD Card Integration implementieren
4. Race Results Screen mit Daten füllen
5. Dokumentation aktualisieren
6. Testing vorbereiten

### To-dos

- [ ] SD Card Integration im DataLoggerService: Datei-Erstellung, CSV-Header, Log-Entries schreiben
- [ ] Race Results Screen mit Daten aus LapCounterService füllen (Leaderboard, Zeiten)
- [ ] Navigation von Race Running Screen zu Results Screen beim Stop implementieren
- [ ] Settings Screen: Teams-Speicherung über PersistenceService implementieren
- [ ] Settings Screen: BLE Settings Dialog/Screen hinzufügen
- [ ] Kompilierung prüfen und Fehler beheben
- [ ] UI-Flows testen: Navigation zwischen allen Screens, Touch-Interaktionen