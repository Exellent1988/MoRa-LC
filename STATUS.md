# MoRa-LC - Implementierungs-Status

**Letztes Update:** Code-Review & Bugfixes abgeschlossen (ultralight_v3)
**Version:** 3.0.1

## ✅ UltraLight v3 - Fertig & Bugfixed

### Code-Review Ergebnisse (März 2026)

Im Rahmen eines vollständigen Code-Reviews wurden folgende Bugs gefunden und behoben:

**Behobene Bugs:**
1. **KRITISCH: BLE Callback-Überschreibung** – `main.cpp` (Normal Mode) überschrieb den LapCounter-internen BLE-Callback mit einem Debug-Print → Rundenzählung hat nie funktioniert. **Behoben.**
2. **KRITISCH: Doppelte Navigation beim Race-Stop** – `stopBtnEventHandler` rief `stopRace()` (navigiert zu Results) UND danach noch `goBack()` auf → sofortige Weiternavigation weg von Results. **Behoben.**
3. **Rennen Auto-Stop fehlte** – Race Running Screen hat die Restzeit zwar angezeigt, aber nie automatisch gestoppt wenn 0:00 erreicht. **Behoben (Auto-Stop + Weiterleitung zu Results).**
4. **Teams-Screen Delete ohne Bestätigung** – Langer Druck auf ein Team hat es sofort gelöscht ohne Dialog. **Behoben (DialogManager Confirm-Dialog).**
5. **SdFat/SD.h Konflikt** – `data_logger_service.cpp` inkludierte sowohl `<SdFat.h>` als auch `<SD.h>`. **Behoben.**
6. **BLE Scan startet nicht ohne Rennen** – Im Normal Mode wurde der Scan nie gestartet (kein `startScanning()` nach `beaconService.begin()`). **Behoben.**

### Feature-Stand

- ✅ **Clean Architecture** – Modulare Service-basierte Architektur
- ✅ **Core System** – SystemManager, TaskManager, MemoryManager
- ✅ **Hardware Abstraction** – Display/LVGL, Touch, SD Card, BLE
- ✅ **Services:**
  - `BeaconService` – BLE Scanning mit NimBLE, RSSI-Mittelwert, Race-Mode
  - `LapCounterService` – Automatische Rundenzählung (RSSI Near/Far), Team-Management, Persistence-Integration
  - `PersistenceService` – NVS-basierte Datenspeicherung (Teams bleiben nach Reboot erhalten)
  - `DataLoggerService` – SD Card CSV Logging mit Timestamp, Auto-Flush, Datei-Management
- ✅ **UI System (LVGL 8):**
  - Home Screen (Hauptmenü)
  - Teams Screen (Liste, Hinzufügen, Löschen mit Bestätigung)
  - Team Edit Screen (On-Screen Keyboard)
  - Beacon Assign Screen (RSSI-basierte Auswahl)
  - Race Setup Screen (Dauer 5–120 Min)
  - Race Running Screen (Timer, Restzeit, Live-Leaderboard, Auto-Stop)
  - Race Results Screen (Rangliste mit Rundenzeiten)
  - Settings Screen (Teams speichern, BLE-Info, Reset)
  - BLE Test Screen (Debug-Ansicht)
- ✅ **Navigation** – Vollständiges Stack-basiertes Navigation-System
- ✅ **BLE Integration** – NimBLE (Standard), Duplicate Filter disabled, MAC-Filter
- ✅ **SD Card Logging** – CSV mit Header, periodisches Flush
- ✅ **Persistenz** – Teams & RSSI-Settings in NVS
- ✅ **Kompilierung** – Fehlerfrei, keine Warnings
  - RAM: 31.3% (102 KB / 327 KB)
  - Flash: 75.5% (989 KB / 1310 KB)

### UI-Flow Review

| Flow | Status | Anmerkungen |
|------|--------|-------------|
| Home → Race Setup → Race Running → Race Results | ✅ | Auto-Stop, korrektes Navigations-Verhalten |
| Home → Teams → Add Team → Edit Name → Back | ✅ | On-Screen Keyboard, Validierung |
| Home → Teams → Team → Assign Beacon | ✅ | BLE Scan, RSSI-Auswahl |
| Home → Teams → Long Press → Delete Confirm | ✅ | DialogManager Confirm |
| Home → Results (letzte Ergebnisse) | ✅ | Zeigt Leaderboard aus LapCounter |
| Home → Settings → Save Teams | ✅ | NVS via PersistenceService |
| Home → Settings → BLE Settings | ✅ | Info-Dialog mit Parametern |
| Home → Settings → Reset | ✅ | Confirm-Dialog, löscht NVS |
| Race Running → Pause/Resume | ✅ | Timer pausiert korrekt |
| Race Running → Auto-Stop bei Zeit=0 | ✅ | Navigiert automatisch zu Results |

### BLE Scan-Parameter

| Parameter | Wert | Anmerkung |
|-----------|------|-----------|
| Library | NimBLE-Arduino | Beste Duplicate-Handling |
| Scan Interval | 100 ms | |
| Scan Window | 99 ms | Fast-continuous |
| Duplicate Filter | false | Nötig für RSSI-Updates im Race Mode |
| RSSI Threshold | -100 dBm | Kein Filter (alles wird gezählt) |
| MAC Filter | `c3:00:` | Xiaomi Mi Beacon Prefix |
| Lap RSSI Near | -65 dBm | Beacon "da" (vorbeigefahren) |
| Lap RSSI Far | -80 dBm | Beacon "weg" |
| Min Lap Time | 10.000 ms | Anti-Bounce |

### Farb-Kontraste (WCAG-Prüfung)

| Paar | Verhältnis | WCAG AA |
|------|-----------|---------|
| TEXT auf BACKGROUND | 12.9:1 | ✅ PASS |
| TEXT auf SURFACE | 21.0:1 | ✅ PASS |
| HEADER_TEXT auf HEADER_BG | 8.6:1 | ✅ PASS |
| BUTTON_TEXT auf BUTTON | 5.4:1 | ✅ PASS |
| BUTTON_TEXT auf SECONDARY | 15.3:1 | ✅ PASS |
| BUTTON_TEXT auf DANGER | 5.3:1 | ✅ PASS |

---

## 🚧 Ausstehend

### UltraLight v3 – Noch offen (nach Field-Test entscheiden)

- [ ] **Hardware-Test** – Noch kein echter Test auf Hardware durchgeführt
- [ ] **Touch-Kalibrierung** – Eventuell nötig (Kalibrierungsdaten in `config.h` vorhanden)
- [ ] **RSSI-Kalibrierung** – Lap-Detection Schwellwerte hängen von Umgebung ab
- [ ] **Icon System** – Aktuell LVGL Built-in Symbole (LV_SYMBOL_*), keine eigenen Bitmaps
- [ ] **Scroll in langen Team-Listen** – LVGL List scrollt, aber bei >8 Teams evtl. träge
- [ ] **Beacon-MAC-Zuordnung robuster machen** – Aktuell: Extraktion aus Label-Text (fragil)

### FullBlown System (noch nicht begonnen)

**Firmware (0%):**
- ❌ Beacon-Firmware (ESP32 + LoRa + BLE + IMU)
- ❌ Gateway-Firmware (LoRa RX + MQTT + BLE RSSI)
- ❌ Display-Firmware (WS2812 + LoRa)

**Backend (0%):**
- ❌ FastAPI Setup
- ❌ MQTT Client & Handler
- ❌ SQLite Datenbank
- ❌ Position Tracker (RSSI Trilateration)
- ❌ Services (Lap Counter, Crash Detector)

**Frontend (0%):**
- ❌ React + Vite Setup
- ❌ Admin Dashboard
- ❌ Live-View (Leaflet Karte)
- ❌ WebSocket Integration

---

## 📊 Gesamt-Fortschritt

| Komponente | Fortschritt | Status |
|------------|------------|--------|
| UltraLight v3 Firmware | 95% | Bereit für Hardware-Test |
| LoRa Protokoll | 100% | Definiert & dokumentiert |
| FullBlown Firmware | 0% | Nicht begonnen |
| FullBlown Backend | 0% | Nicht begonnen |
| FullBlown Frontend | 0% | Nicht begonnen |
| **Gesamt** | **~40%** | |

---

## 🎯 Nächste Schritte

### Priorität 1: UltraLight Field-Test

**Hardware besorgen:**
- 1x CheapYellow Display (ESP32-2432S028) – ~15€
- 3–5x Xiaomi Mi Beacon – ~9–15€
- 1x MicroSD-Karte (FAT32, ≤32GB) – ~5€
- 1x USB Powerbank (≥5000mAh) – ~15€

**Firmware flashen:**
```bash
cd firmware
pio run -e ultralight_v3 -t upload
# Oder mit Monitor:
pio run -e ultralight_v3 -t upload && pio device monitor
```

**Test-Szenario:**
1. 3 Teams anlegen (On-Screen Keyboard)
2. Beacons per RSSI zuordnen (Beacon nah an Display halten)
3. Kurzes Testrennen starten (5 Min)
4. Kontrollieren: Rundenzählung korrekt? SD-Log vorhanden?
5. Ergebnisse ansehen

**Zu prüfen:**
- Touch-Responsivität (ggf. Kalibrierung anpassen)
- BLE-Scan-Reichweite (funktioniert Lap-Detection bei realer Vorbeifahrt?)
- RSSI-Schwellwerte (Anpassung in `config.h`: `DEFAULT_LAP_RSSI_NEAR` / `DEFAULT_LAP_RSSI_FAR`)
- SD-Logging (Datei `/races/race_<timestamp>.csv` auf SD-Karte?)

### Priorität 2: FullBlown Basis (nach erfolgreichem Field-Test)

1. Beacon-Firmware (LoRa + BLE + IMU)
2. Gateway-Firmware (LoRa RX + MQTT Bridge)
3. Python Backend Basis (FastAPI + MQTT + DB)
4. Frontend Setup (React + Basic UI)

---

## 🐛 Bekannte Einschränkungen

| Problem | Schwere | Workaround |
|---------|---------|------------|
| Kein Hardware-Test | Hoch | Bald testen |
| Beacon-MAC aus Label-Text (fragil) | Mittel | Funktioniert solange Format gleich bleibt |
| Kein RTC – Timestamps relativ zu millis() | Niedrig | Für Renndauer ausreichend |
| Max. 20 Teams (konfigurierbar in `config.h`) | Niedrig | Für meisten Anwendungsfälle genug |
| Fonts: Umlaute in Standard-ASCII-Font nicht sicher | Niedrig | Alle Labels auf ASCII geändert |

---

## 📁 Datei-Statistik (ultralight_v3)

| Kategorie | Dateien | Zeilen |
|-----------|---------|--------|
| Core (config, system) | 6 | ~300 |
| Hardware (display, BLE, SD, touch) | 10 | ~600 |
| Services (beacon, lap, logger, persist) | 8 | ~800 |
| UI Screens (LVGL) | 18 | ~1400 |
| UI Widgets (dialog, button, icons) | 6 | ~400 |
| **Gesamt** | **~48** | **~3500** |

**Build-Status:** ✅ `ultralight_v3` kompiliert fehlerfrei (0 Errors, 0 Warnings)

---

*Nächstes Update nach Hardware-Test*
