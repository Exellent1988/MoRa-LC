# UltraLight - Build & Upload Guide

## Hardware

- **CheapYellow Display (ESP32-2432S028)**
  - ESP32 mit integriertem 2.8" TFT Display (320x240)
  - Resistiver Touchscreen
  - MicroSD Card Slot
  - USB-C für Programmierung

- **BLE iBeacons**
  - Beliebige BLE iBeacons (z.B. ESP32-C3 mit iBeacon Firmware)
  - Magnete oder Kabelbinder zur Befestigung am Moped

## Software Requirements

- **PlatformIO CLI** oder **PlatformIO in VS Code/Cursor**
- **USB Treiber** für ESP32 (CP210x oder CH340)

## Installation

### 1. PlatformIO installieren

**VS Code / Cursor:**
```bash
# PlatformIO Extension installieren
# Suche nach "PlatformIO IDE" im Extensions Marketplace
```

**CLI:**
```bash
pip install platformio
```

### 2. Dependencies installieren

```bash
cd firmware
pio pkg install
```

## Build & Upload

### Option 1: PlatformIO CLI

```bash
cd firmware

# Build für UltraLight
pio run -e ultralight

# Upload auf ESP32
pio run -e ultralight -t upload

# Serial Monitor öffnen
pio device monitor -e ultralight
```

### Option 2: VS Code / Cursor

1. Öffne `firmware/` Ordner in VS Code/Cursor
2. PlatformIO Sidebar öffnen (Alien-Icon)
3. Environment: `ultralight` auswählen
4. **Build** klicken (oder `Cmd/Ctrl + Alt + B`)
5. ESP32 per USB anschließen
6. **Upload** klicken (oder `Cmd/Ctrl + Alt + U`)
7. **Serial Monitor** öffnen (oder `Cmd/Ctrl + Alt + S`)

### Troubleshooting

**Upload funktioniert nicht:**
```bash
# Port manuell angeben
pio run -e ultralight -t upload --upload-port /dev/cu.usbserial-*

# Oder auf macOS:
ls /dev/cu.*

# Auf Linux:
ls /dev/ttyUSB*
```

**Display bleibt schwarz:**
- Board neu starten (RST Button)
- USB-Kabel prüfen (manche Kabel sind nur zum Laden)
- TFT_eSPI Konfiguration prüfen in `platformio.ini`

**Touch funktioniert nicht:**
- Kalibrierung ggf. anpassen in TFT_eSPI
- Touchscreen ist resistiv - mit Fingernagel oder Stylus testen

## First Boot

### 1. Setup-Bildschirm

Beim ersten Start erscheint der Setup-Bildschirm:
```
MoRa-LC UltraLight v0.1.0
Initializing...
BLE: OK
SD: OK
Loading Teams...
```

### 2. Hauptmenü

Nach dem Booten siehst du 4 Buttons:
- **Neues Rennen** - Startet die Rennen-Vorbereitung
- **Teams verwalten** - Teams hinzufügen, bearbeiten, Beacons zuordnen
- **Ergebnisse** - Zeigt Ergebnisse des letzten Rennens
- **Einstellungen** - System-Einstellungen (noch nicht implementiert)

### 3. Teams anlegen

1. **Teams verwalten** antippen
2. **+ Neues Team** antippen
3. Team-Namen bearbeiten (Keyboard erscheint)
4. Beacon zuordnen:
   - **Beacon zuordnen** antippen
   - Beacon in die Nähe (<1m) halten
   - Display zeigt erkannte Beacons mit Distanz
   - **Zuordnen** antippen wenn gewünschter Beacon grün angezeigt wird
5. **Speichern** antippen

💡 **Tipp:** Teams werden automatisch in NVS gespeichert und bleiben nach Neustart erhalten!

### 4. Rennen starten

1. **Neues Rennen** antippen
2. Rennen-Name eingeben (z.B. "Training 2025-11-01")
3. Dauer in Minuten eingeben (Standard: 60 min)
4. **Rennen starten** antippen

Das System:
- Startet BLE Scanning automatisch
- Zählt Runden wenn Beacons in Reichweite kommen
- Zeigt Live-Rangliste
- Speichert Daten auf SD-Karte (falls vorhanden)

### 5. Während des Rennens

**Display zeigt:**
- Verbleibende Zeit (groß, oben)
- Rangliste (Team, Runden, letzte Rundenzeit)
- Buttons: Pause / Stop

**Rundenzählung:**
- Beacon muss <3m zum Display kommen
- Mindestzeit zwischen Runden: 10 Sekunden
- Piepton bei erkannter Runde (wenn implementiert)

### 6. Nach dem Rennen

**Ergebnisse:**
- Rangliste nach Runden (dann nach Zeit)
- Beste Rundenzeit pro Team
- Durchschnittliche Rundenzeit

**SD-Karte Daten:**
```
/races/
  rennen_2025-11-01_14-30.csv
  rennen_2025-11-01_16-00.csv
```

CSV Format:
```csv
timestamp,team_id,team_name,lap,lap_time,total_time
1234567,1,Team Alpha,1,65432,65432
1234567,1,Team Alpha,2,63210,128642
...
```

## Configuration

### BLE Scanner Settings

In `firmware/ultralight/src/config.h`:

```cpp
// BLE Scanner
#define BLE_SCAN_INTERVAL 100        // ms
#define BLE_SCAN_WINDOW 99           // ms
#define BLE_RSSI_THRESHOLD -80       // dBm (Signal-Stärke Filter)
#define BLE_PROXIMITY_THRESHOLD 3.0  // meters (Runden-Erkennungs-Distanz)
#define BEACON_TIMEOUT 5000          // ms (Beacon als "weg" markieren)

// Race Settings
#define MIN_LAP_TIME 10000           // ms (Mindest-Zeit zwischen Runden)
#define MAX_TEAMS 20                 // Maximale Anzahl Teams
#define MAX_RACE_DURATION 7200000    // ms (2 Stunden max)
```

### Display Colors

```cpp
#define COLOR_PRIMARY 0x0014       // Dark Blue
#define COLOR_SECONDARY 0x07E0     // Green
#define COLOR_DANGER 0xF800        // Red
#define COLOR_WARNING 0xFD20       // Orange
#define COLOR_BUTTON 0x5AEB        // Light Gray
```

## Debugging

### Serial Monitor

```bash
pio device monitor -e ultralight -b 115200
```

Output:
```
[Display] Initializing...
[Display] Initialized
[BLE] Initializing...
[BLE] Initialized
[SD] Initializing...
[SD] Initialized
[Persistence] Initializing...
[Persistence] Loaded 3 teams

[Touch] X=120, Y=80, Screen=0
[Beacon] Unknown: 12:34:56:78:9A:BC (RSSI: -65, Dist: 1.23m)
[Lap] Team 1 (Team Alpha): Runde 1
```

### Common Issues

**BLE Scan findet keine Beacons:**
- iBeacon UUID prüfen (muss mit Beacon übereinstimmen)
- RSSI Threshold anpassen (höher = empfindlicher)
- Beacon Batterie prüfen

**SD-Karte funktioniert nicht:**
- FAT32 formatieren
- CS Pin prüfen (config.h: `SD_CS_PIN 5`)
- Karte beim Boot eingelegt lassen

**Teams verschwinden nach Neustart:**
- NVS wird nicht gelöscht, außer bei Flash-Erase
- Persistence Logs im Serial Monitor prüfen
- Falls nötig: `pio run -e ultralight -t erase` (löscht ALLES!)

## Advanced

### Custom iBeacon UUID

In `firmware/lib/BLEScanner/BLEScanner.h`:

```cpp
// Eigene UUID generieren: https://www.uuidgenerator.net/
#define CHECKPOINT_UUID_START_FINISH "E2C56DB5-DFFB-48D2-B060-D0F5A71096E0"
```

### Over-The-Air (OTA) Updates

TODO: OTA Update über WiFi implementieren

## Support

Bei Problemen:
1. Serial Monitor Output kopieren
2. Issue auf GitHub erstellen
3. Hardware-Fotos anhängen (falls Hardware-Problem)

Happy Racing! 🏁



