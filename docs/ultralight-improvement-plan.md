# UltraLight (CYD) – Fix- & Verbesserungsplan

Stand: Analyse von `firmware/src/ultralight_v2` + `lib/` (Default-Env `ultralight_v2`).

## 1. Gefundene Ursachen (fehlende Runden / nichts auf SD)

| # | Problem | Wo | Folge |
|---|---------|----|-------|
| B1 | **SPI-Konflikt SD ↔ Display**: Display nutzt `VSPI_HOST` (Pins 13/14), SD nutzt globales `SPI` = ebenfalls VSPI (18/19/23). LovyanGFX übernimmt den Bus nach `SD.begin()`. | `LGFX_Config.hpp:23`, `DataLogger::begin` | SD-Schreibzugriffe schlagen fehl/sporadisch → keine Runden auf SD |
| B2 | **Lap-Logik läuft im NimBLE-Task**: `onBeaconDetected` → `recordLap` + SD-Write direkt im BLE-Callback. Blockiert den BLE-Host (SD-Write = 10–100 ms) → Advertisements gehen verloren. | `main.cpp:onBeaconDetected` | Bei 10 Beacons viele verpasste Pakete → verpasste Runden |
| B3 | **Race Conditions**: `std::map beacons` wird im BLE-Task beschrieben und im Loop gelesen/gelöscht (`getBeacons`, `clearOldBeacons`); `TeamData`/`laps` ebenso (UI zeichnet während BLE schreibt). Kein Mutex. | `BLEScanner.cpp`, `LapCounter` | Heap-Korruption, Hänger, Reboots |
| B4 | **Hysterese-Bug**: „weg“ wird nur gesetzt, wenn ein Paket mit RSSI < `far` (-80) ankommt. Verschwindet der Beacon einfach (Timeout), bleibt `beaconPresence = true` → **alle folgenden Runden werden ignoriert**. Gleiches, wenn die Strecke nie unter -80 fällt. | `main.cpp:onBeaconDetected` | Systematisch fehlende Runden für einzelne Teams |
| B5 | **Einzel-Sample-Trigger**: Runde zählt nur, wenn *ein* Paket > -65 dBm. Bei schneller Durchfahrt + Antennenabschattung (Fahrer/Körper) wird der Peak oft nicht erreicht. | dito | Fehlende Runden |
| B6 | Active Scan + 10 Beacons = doppelter Funkverkehr (Scan-Responses), nicht nötig für iBeacon/MAC-Erkennung. | `BLEScanner::begin` | Weniger empfangene Adverts |
| B7 | `MIN_LAP_TIME` (10 s) aus config wird nicht genutzt, LapCounter hat hart 5 s. | `LapCounter.cpp` | Inkonsistent |
| B8 | SD: Datei pro Runde öffnen/schließen, kein Fehler-Feedback im UI, keine Wiederholung; Dateiname ohne echte Uhrzeit. Keine Rohdaten. | `DataLogger` | Fehler fallen erst nach dem Rennen auf |
| B9 | Kein Persistieren des Rennstands → Reboot = alle Runden weg. | – | Datenverlust |

## 2. Fix-Plan (Priorität)

**Phase 1 – Zuverlässigkeit (zuerst, klein & testbar)**
1. **SPI fixen**: Display auf `HSPI_HOST` (CYD-Standard), SD auf eigene `SPIClass(VSPI)` mit Pins 18/19/23/5; Touch per Bitbang/Software-SPI oder geteilter Bus. Beim Boot SD-Selbsttest (write/read) → Anzeige rot im UI.
2. **Entkoppeln**: BLE-Callback macht nur `xQueueSend({mac, rssi, millis})` (kein String, kein Map-Zugriff). Auswertung + SD im Loop/eigenem Task. Damit erledigt sich B3 größtenteils; Rest mit Mutex.
3. **Neue Lap-Erkennung (State Machine pro Team)**:
   - Glättung (z. B. Median/EMA über ~3 Samples).
   - `AWAY → NEAR` wenn geglätteter RSSI > near **oder** Peak-Erkennung (Maximum der Durchfahrt).
   - `NEAR → AWAY` wenn RSSI < far **oder kein Paket seit X s (Timeout)** ← fixt B4.
   - Rundenzeit = Zeitpunkt des RSSI-Peaks (genauer als „erstes Paket über Schwelle“).
   - Mindestrundenzeit aus Config (B7).
4. **Passive Scan**, Interval = Window (100/100), Duplicate-Filter aus. Beacons auf 100 ms Advertising-Intervall + max. TX-Power stellen.
5. **SD robust**: Datei während des Rennens offen halten, `flush()` nach jeder Runde, Fehlerzähler + UI-Warnung, Ringpuffer im RAM für Retry.
6. **Crash-Recovery**: Rennstand (Laps) zusätzlich in NVS/SD-Journal, beim Boot „Rennen fortsetzen?“.

**Phase 2 – Testbarkeit**
1. **Rohdaten-Log** (`raw_*.csv`: ms, mac, rssi) auf SD während des Rennens → Rennen offline nachspielen.
2. **Lap-Logik als reine C++-Klasse** ohne Arduino-Abhängigkeit → Unit-Tests mit `pio test -e native` (Env existiert schon).
3. **Replay-Tool** (Python): Rohlog einlesen, Lap-Algorithmus mit verschiedenen Schwellen simulieren, Vergleich mit Soll-Runden.
4. **Simulator-Firmware**: zweites ESP32 sendet N Fake-Beacons mit RSSI-Profil (Durchfahrt) → reproduzierbarer Lasttest mit 10–20 Beacons.
5. **Diagnose-Screen**: pro Team letzter RSSI, Pakete/s, Zustand (NEAR/AWAY), letzte Runde.

**Phase 3 – Komfort**
- Uhrzeit per NTP/RTC (DS3231 ~2 €) für echte Dateinamen/Zeitstempel.
- Webserver/CSV-Download (siehe `ultralight-webserver-plan.md`).
- Manuelles +1/-1 pro Team im UI (Korrektur).

## 3. Separater Task: echtes Tracking mit Karte

| Option | Kosten/Fahrzeug | Genauigkeit | Infrastruktur | Bewertung |
|--------|-----------------|-------------|---------------|-----------|
| **GPS + LoRa** (ESP32 + ATGM336H/NEO-M8 + SX1276) | ~12–18 € | 2–3 m (M8/M10 mit 10 Hz besser) | 1 Gateway | **Empfehlung** für Outdoor-Strecken. Karte trivial (Lat/Lon), passt zum geplanten FullBlown-LoRa-Konzept. Runden per virtueller Ziellinie (Geofence). |
| GPS + ESP-NOW (Long Range) | ~8–12 € | wie oben | 1 ESP-Empfänger, evtl. Repeater | Noch billiger, Reichweite ~200–500 m mit LR-Modus. Gut für kleine Strecken. |
| **UWB DW3000** (TWR/TDoA) | ~10–15 € Tag + 4–8 Anker á ~15 € | 10–30 cm | Anker mit Strom entlang Strecke, Zeitsync | Sehr genau, aber Aufwand: Reichweite ~50–100 m Sichtlinie, viele Tags → Update-Rate sinkt (TWR), TDoA braucht Anker-Sync. Lohnt nur für kleine/abgeschattete (Indoor-)Strecken. |
| BLE-RSSI Trilateration mit vorhandenen ESPs | ~0 € | 5–15 m | viele ESP-Empfänger | Billig, aber zu ungenau für Karte. Taugt nur für „Sektor-Erkennung“. |

**Vorschlag:** GPS (10 Hz, M10-Modul) + LoRa/ESP-NOW für Position und Karte; BLE-Ziellinie (die bestehende CYD-Lösung) bleibt als präziser Rundenzähler/Backup. Die vorhandenen ESPs können als Sender (am Fahrzeug) und als Gateway dienen; BLE-Beacons bleiben für die Ziellinie.

Offene Fragen für die Auswahl: Streckenlänge/-form, Indoor oder Outdoor, wie viele Fahrzeuge max., gewünschte Update-Rate auf der Karte.

## 4. Umsetzungsstand

Umgesetzt (kompiliert mit `pio run -e ultralight_v2`, **noch nicht auf Hardware getestet**):

- B1: Display ohne MISO-Pin (`LGFX_Config.hpp`), SD-Selbsttest nach Display-Init + Re-Init bei Fehler.
- B2/B3: BLE-Callback legt nur noch Roh-Pakete in eine Queue (`BLEScanner::popSample`); Auswertung in `loop()`; Beacon-Liste per Mutex, waehrend des Rennens abgeschaltet.
- B4/B5/B7: neue `LapDetector`-Klasse (Glaettung, Peak-Zeitstempel, Timeout fuer "weg", Mindestrundenzeit aus Config).
- B6: Passiver Scan, Interval = Window, nicht-blockierender Scan-Start, Neustart wenn Scan stoppt.
- B8: SD-Dateien bleiben offen, `flush()` pro Runde, Recover bei Schreibfehler, Fehleranzeige im Rennscreen, Dateikollisionen nach Reboot behoben.
- Phase 2: Roh-Log `*_raw.csv`, native Tests (`firmware/tools/run_native_tests.sh`), Replay-Tool (`firmware/tools/replay/replay.cpp`), Diagnose-Zeile alle 5 s im Serial.

Offen: B9 (Crash-Recovery), Diagnose-Screen im UI, Beacon-Simulator, RTC/NTP.
