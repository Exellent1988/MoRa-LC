# MoRa-LC - Implementierungs-Status

**Letztes Update:** Hardware-Test & Optimierungen abgeschlossen (ultralight_v3)
**Version:** 3.1.0

## UltraLight v3 - Hardware-getestet & optimiert

### Hardware-Test Ergebnisse (März 2026)

Erfolgreich auf CheapYellow Display (ESP32-2432S028) mit Xiaomi Mi Beacons getestet.
Mehrere Rennen mit 3 Teams simuliert, Rundenzählung und CSV-Logging verifiziert.

### Behobene Bugs & Verbesserungen

**Kritische Fixes:**
1. **Touch-Input funktionierte nicht** – SPI-Konflikt zwischen Display und Touch. Touch auf Software-SPI (Bit-Banging) umgestellt, Display auf HSPI. **Behoben.**
2. **BLE-Scan blockierend** – `NimBLEScan::start()` mit 2 Parametern blockiert. Auf non-blocking 3-Parameter-Variante umgestellt. **Behoben.**
3. **Race Results zeigt 0 Runden** – `stopRace()` → `setScreen(results)` → `onExit()` → `stopRace()` → infinite Recursion → ESP32 Crash → Reboot → RAM-Daten verloren. **Behoben: `onExit()` bereinigt nur State, navigiert nicht.**
4. **SD-Karte leer** – SPI-Bus-Konflikt Display/SD-Karte. Display auf HSPI, SD auf VSPI. **Behoben.**
5. **Erster Lap sofort bei Rennstart gezählt** – `_beaconPresence` nicht vorgeladen. Fix: Alle Beacons bei `startRace()` als "present" vorinitialisiert. **Behoben.**
6. **Alle Teams zeigen gleiche MAC** – String-Truncation bei MAC-Anzeige. Fix: Nur letzte 5 Zeichen anzeigen. **Behoben.**

**UI/UX Verbesserungen:**
- Keyboard-Gap zwischen Eingabefeld und Tastatur geschlossen (`LV_ALIGN_TOP_LEFT`)
- Race Running Screen: Zwei-View-System (Timer / Leaderboard) mit Toggle-Button
- Settings: Min Lap Time, RSSI Near/Far über Modal-Dialog mit +/- Buttons einstellbar
- Race Setup: Rennname-Eingabe, +15min Button, Overlay-Keyboard
- Race Results: 3-Ebenen-Navigation (Rennliste → Rennergebnis → Team-Rundenzeiten)
- Debug Screen: Live-Ansicht aller Beacons mit Raw/Avg RSSI und Near/Far/Mid Status (farbcodiert)
- BLE Settings Dialog: Lesbare Größe, alle Parameter sichtbar

**Performance-Optimierungen:**
- BLE Scan: Passive Scan (weniger Latenz), Interval 50ms/Window 49ms
- RSSI History: 5 → 3 Samples (schnellere Reaktion)
- Near-Detection: Raw RSSI statt Average (sofortige Erkennung bei Vorbeifahrt)
- Far-Detection: Average RSSI (stabiler, keine Fehlauslösung)
- Hysteresis-Logik: Near/Far Schwellwerte verhindern Flapping

### Feature-Stand

- **Clean Architecture** – Modulare Service-basierte Architektur
- **Core System** – SystemManager, TaskManager, MemoryManager
- **Hardware Abstraction** – Display/LVGL, Touch (Software-SPI), SD Card, BLE
- **Services:**
  - `BeaconService` – BLE Scanning mit NimBLE, 3-Sample RSSI-Mittelwert, Race-Mode
  - `LapCounterService` – Hysteresis-Rundenzählung (Raw RSSI Near / Avg RSSI Far), Team-Management, Persistence
  - `PersistenceService` – NVS-basierte Datenspeicherung (Teams, RSSI-Settings, Min Lap Time)
  - `DataLoggerService` – SD Card CSV Logging mit Race-History, Datei-Listing, CSV-Parsing
- **UI System (LVGL 8):**
  - Home Screen (Hauptmenü)
  - Teams Screen (Liste, Hinzufügen, Löschen mit Bestätigung)
  - Team Edit Screen (Overlay On-Screen Keyboard)
  - Beacon Assign Screen (RSSI-basierte Auswahl, letzte 5 MAC-Zeichen)
  - Race Setup Screen (Dauer 5–180 Min, Rennname-Eingabe, Overlay-Keyboard)
  - Race Running Screen (Timer/Leaderboard Toggle, Elapsed/Remaining, Auto-Stop)
  - Race Results Screen (3-Ebenen: Rennliste → Leaderboard → Team-Laps aus CSV)
  - Settings Screen (Min Lap Time, RSSI Near/Far via +/- Dialog, Debug, BLE Info, Reset)
  - Debug Screen (Live Beacon RSSI, Near/Far/Mid farbcodiert, Lap Count)
- **Navigation** – Stack-basiert, Back-Button auf allen Screens
- **BLE Integration** – NimBLE, Passive Scan, Duplicate Filter disabled, MAC-Filter
- **SD Card Logging** – CSV pro Rennen mit Rennname, Rundenzeiten, RSSI, Time-of-Day
- **Race History** – CSV-Dateien von SD lesen, per Rennen und Team abrufbar
- **Persistenz** – Teams, RSSI Near/Far, Min Lap Time in NVS

### UI-Flow

| Flow | Status | Anmerkungen |
|------|--------|-------------|
| Home → Race Setup → Race Running → Stop → Results → Home | OK | Kein Recursion-Bug mehr |
| Home → Results → Rennen wählen → Team wählen → Rundenzeiten | OK | 3-Ebenen CSV-basiert |
| Home → Teams → Add → Keyboard → Back | OK | Overlay-Keyboard |
| Home → Teams → Team → Assign Beacon | OK | RSSI-Sortierung, Short-Click |
| Home → Teams → Long Press → Delete Confirm | OK | DialogManager |
| Home → Settings → Min Lap Time / RSSI → +/- Dialog | OK | Persistiert in NVS |
| Home → Settings → Debug → Live RSSI | OK | Farbcodiert Near/Far/Mid |
| Race Running → Timer/Board Toggle | OK | Große Uhr vs. Leaderboard |
| Race Running → Auto-Stop bei Zeit=0 | OK | → Results |

### BLE Scan-Parameter

| Parameter | Wert | Anmerkung |
|-----------|------|-----------|
| Library | NimBLE-Arduino | Passive Scan |
| Scan Interval | 50 ms | |
| Scan Window | 49 ms | Fast-continuous |
| Scan Mode | Passiv | Weniger Latenz als Active |
| Duplicate Filter | false | Nötig für RSSI-Updates |
| RSSI Threshold | -100 dBm | Filter in BLE Layer |
| MAC Filter | `c3:00:` | Xiaomi Mi Beacon Prefix |
| RSSI History | 3 Samples | Circular Buffer |

### Lap Detection Parameter (konfigurierbar via UI)

| Parameter | Default | Range | Anmerkung |
|-----------|---------|-------|-----------|
| RSSI Near | -65 dBm | -30 bis -90 | Raw RSSI >= Near → "present" (fast trigger) |
| RSSI Far | -80 dBm | -40 bis -100 | Avg RSSI <= Far → "absent" (smooth) |
| Min Lap Time | 10 s | 1–120 s | Debounce gegen Doppelzählung |

### Build-Status

- RAM: 31.5% (103 KB / 327 KB)
- Flash: 84.3% (1104 KB / 1310 KB)
- Kompilierung: 0 Errors, 0 Warnings

---

## Ausstehend

### UltraLight v3 – Optional (nach Field-Test)

- [ ] **RSSI-Kalibrierung** – Schwellwerte je nach Strecke/Umgebung anpassen
- [ ] **Wokwi Simulator** – Für UI-Entwicklung ohne Hardware
- [ ] **Race Running: Größere Uhr** – Font-Limitierung bei Flash-Größe
- [ ] **Current Race in Results** – Auch ohne CSV (nur aus RAM) Rundenzeiten anzeigen
- [ ] **Active Scan Fallback** – Falls Passive Scan iBeacon-Daten nicht liefert

### FullBlown System (noch nicht begonnen)

**Firmware (0%):**
- Beacon-Firmware (ESP32 + LoRa + BLE + IMU)
- Gateway-Firmware (LoRa RX + MQTT + BLE RSSI)
- Display-Firmware (WS2812 + LoRa)

**Backend (0%):**
- FastAPI Setup, MQTT Client, SQLite, Position Tracker

**Frontend (0%):**
- React + Vite, Admin Dashboard, Live-View (Leaflet), WebSocket

---

## Gesamt-Fortschritt

| Komponente | Fortschritt | Status |
|------------|------------|--------|
| UltraLight v3 Firmware | 98% | Hardware-getestet |
| LoRa Protokoll | 100% | Definiert |
| FullBlown Firmware | 0% | Nicht begonnen |
| FullBlown Backend | 0% | Nicht begonnen |
| FullBlown Frontend | 0% | Nicht begonnen |
| **Gesamt** | **~40%** | |

---

## Bekannte Einschränkungen

| Problem | Schwere | Workaround |
|---------|---------|------------|
| Kein RTC – Timestamps relativ zu millis() | Niedrig | Für Renndauer ausreichend |
| Max. 20 Teams (konfigurierbar) | Niedrig | Für meisten Rennen genug |
| Fonts: Keine Umlaute | Niedrig | Labels auf ASCII |
| Flash bei 84% | Mittel | Keine weiteren großen Fonts aktivieren |
| Passive Scan: iBeacon-Daten evtl. nur in Scan Response | Niedrig | Fallback auf Active Scan möglich |

---

*Nächstes Update nach Feld-Einsatz bei echtem Rennen*
