<!-- bdea7f97-3364-436b-a304-c3d6473beedf 4422f698-9991-4190-9372-a387ad5239c5 -->
# MoRa-LC: Mofarennen Lap Counter & Timing System

## Übersicht

Das Projekt bietet **zwei System-Varianten** für unterschiedliche Anforderungen:

### Variante 1: **FullBlown** (~460€ für 20 Beacons)

- Raspberry Pi + Web-App + LoRa + BLE RSSI Triangulation
- Live-Tracking auf Karte, Cloud-Streaming, IMU-Sturzerkennung
- Für professionelle Events mit bis zu 20 Teams
- 75x75m Streckenabdeckung

### Variante 2: **UltraLight** (~90€ für 20 Beacons)  

- CheapYellow Touch-Display als Master + Checkpoint in einem
- Simple BLE Beacons (Xiaomi, ~3€) an Mofas
- Rundenzählung + Zeiten per Touch-UI
- Perfekt für kleine lokale Rennen, kein Laptop nötig

Beide Systeme teilen gemeinsame Code-Module (BLE Scanner, Lap Counter, Data Logger).

---

# FULLBLOWN SYSTEM

## System-Architektur

```
[Mofa-Beacons]                    BLE Checkpoints
   ESP32                          (iBeacon)
   + LoRa --------\                  |
   + BLE Scanner   \                 | (Erkennung)
   + IMU            \                |
   (+ GPS/UWB)       \-----> [LoRa Gateways] --MQTT--> [Zentrale]
                              BLE Scanner                 RPi
                                  |                       |
                           (RSSI Triangulation)     [SD + WebApp]
                                                          |
                                                    [LoRa Sender]
                                                          |
                                                   [WS2812 Display]
```

## Hardware-Komponenten

### 1. Mofa-Beacons (~10-12€/Stück)

- **ESP32 DevKit** (~4€) - BLE + WiFi integriert
- **LoRa Modul** RFM95W/SX1276 (~5€) - Langstrecken-Telemetrie
- **IMU** MPU6050 (~2€) - Sturzerkennung, Fahrerwechsel
- **Piezo Buzzer** (~0.50€) - Lokalisierung/Warnungen
- **LiPo Akku** 1500-2000mAh (~3-5€) - 8-10h Laufzeit
- **Gehäuse** wasserfest mit Magneten (~2€)

**Optional später:**

- GPS Modul NEO-6M (~5-8€) - Absolute Position
- UWB Modul DWM1000 (~15-18€) - cm-genaue Triangulation

### 2. BLE Checkpoints (~3-8€/Stück)

**Option A - Günstig:** Xiaomi Mi Beacon (~3€)

- Nur iBeacon Broadcasting, CR2032 Batterie (Monate)
- Keine Status-Rückmeldung

**Option B - Smart:** ESP32 BLE Beacon (~8€)

- iBeacon + LoRa Status-Meldung
- Durchfahrten loggen

**Anzahl:** 5-10 je nach Strecke (Start/Ziel, Boxengasse, Sektoren)

### 3. LoRa Gateways (~15€/Stück, 3-4 benötigt)

- ESP32 + LoRa Modul + BLE Scanner
- RSSI-basierte Triangulation
- Externe Antenne für 75x75m Abdeckung
- Netzteil oder Powerbank

### 4. Zentrale (~60-80€)

**Raspberry Pi 4 (4GB)** - Empfohlen

- Python Backend (FastAPI)
- SD-Karte Datenspeicherung
- WiFi Hotspot
- MQTT Broker
- LoRa Anbindung (USB oder via Gateway)

### 5. LED-Display Systeme (~15-20€/Stück)

- ESP32 + LoRa Modul (Empfänger)
- WS2812B LED Panels (2x 8x8 oder 16x16)
- Modi: Countdown, Ampel, Warnungen
- Per LoRa von Zentrale gesteuert

## Software-Architektur

### ESP32 Firmware (PlatformIO + Arduino)

```
/firmware/
  platformio.ini          # Multi-Environment Config
  /lib/                   # Shared Libraries
    /LoRaComm/
    /BLEScanner/
    /IMUHandler/
  /beacon/                # Mofa-Beacon Firmware
    src/main.cpp
    platformio.ini
  /gateway/               # Gateway Firmware
    src/main.cpp
  /checkpoint/            # Smart Checkpoint (optional)
    src/main.cpp
  /display/               # LED Display Firmware
    src/main.cpp
```

**Beacon-Firmware Features:**

- LoRa Telemetrie alle 1-2s (BeaconID, Battery, IMU, Checkpoint-ID, RSSI)
- BLE Scanner für iBeacon UUIDs (Checkpoint-Erkennung)
- IMU Sturzerkennung (>2.5G Beschleunigung)
- Deep-Sleep zwischen Übertragungen
- Piezo-Signale (Low-Battery, Disconnect)
- Modular für GPS/UWB später

**Gateway-Firmware Features:**

- LoRa Empfang aller Beacon-Nachrichten
- BLE RSSI Scanning für Triangulation
- MQTT Bridge zur Zentrale (WiFi)
- NTP Zeitstempel-Synchronisation

**Display-Firmware Features:**

- LoRa Kommando-Empfang
- WS2812 Steuerung (FastLED)
- Modi: Countdown, Ampel (Rot/Gelb/Grün), Warnung, Text-Scroll

### Python Backend

**Stack:**

- FastAPI (REST API + WebSocket)
- SQLite (lokale DB)
- Paho-MQTT (Hardware-Kommunikation)
- NumPy/SciPy (RSSI-Triangulation)
- Pandas (Auswertung)
```
/backend/
  /app/
    main.py
    /api/
      teams.py              # Team & Beacon Management
      races.py              # Rennen Start/Stop/Pause
      telemetry.py          # WebSocket Live-Daten
      admin.py              # Admin-Funktionen
      track.py              # Strecken-Setup
    /services/
      lora_handler.py       # LoRa Messages verarbeiten
      position_tracker.py   # Position aus BLE RSSI + Checkpoints
      ble_triangulation.py  # RSSI → Position (Trilateration)
      lap_counter.py        # Rundenzählung (Checkpoint-basiert)
      crash_detector.py     # Sturz aus IMU
      penalty_system.py     # Strafen (Abkürzungen)
      data_logger.py        # SD-Karte Logging
      display_controller.py # LoRa → Display Kommandos
    /models/
      database.py           # SQLAlchemy ORM
      schemas.py            # Pydantic
    /utils/
      mqtt_client.py
      track_geometry.py     # Checkpoint-Positionen
  requirements.txt
  config.yaml
```


**Position-Tracking Logik:**

1. **An Checkpoints:** Präzise bekannt
2. **Zwischen Checkpoints:** BLE RSSI Trilateration von 3+ Gateways

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - RSSI → Distanz (ungenau ±5-10m)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Linear interpolieren entlang Strecke

3. **Fallback:** Letzte Checkpoint-Position + Zeit

**Rundenzählung:**

- Start/Ziel Checkpoint zwingend
- Runde = Durchfahrt Start/Ziel + alle Pflicht-Checkpoints
- Unvollständige Runden → Warnung

### React Frontend

**Stack:**

- React 18 + Vite
- Leaflet (Karten mit OpenStreetMap)
- Socket.io-client (Live-Updates)
- TailwindCSS (Styling)
- Recharts (Diagramme)
```
/frontend/
  /src/
    /components/
      TrackMap.jsx          # Leaflet Karte
      CheckpointEditor.jsx  # Drag-Drop Checkpoints
      LiveTelemetry.jsx
      LapTimeTable.jsx
      TrafficLight.jsx      # Display-Steuerung
      TeamCard.jsx
    /views/
      AdminDashboard.jsx    # Setup, Team-Mgmt, Kontrolle
      LiveView.jsx          # Öffentliche Ansicht
      TeamView.jsx          # Team-Login
      AnalysisView.jsx      # Multi-Rennen Auswertung
    /services/
      api.js
      websocket.js
    App.jsx
  package.json
```


**Admin-Features:**

- Strecken-Setup: Karte hochladen, Checkpoints positionieren
- Beacon-Verwaltung: Team zuordnen, Batterie-Status
- Rennen-Kontrolle: Start/Pause/Stop, Ampel steuern
- Live-Karte: Alle Beacons mit RSSI-Position

## Implementierungs-Phasen

### Phase 1: Basis-System

- PlatformIO Multi-Environment Setup
- LoRa Protokoll definieren
- Beacon-Firmware: LoRa Telemetrie
- Gateway-Firmware: LoRa → MQTT
- Python Backend: FastAPI + MQTT + DB
- React Frontend: API Integration

### Phase 2: BLE Checkpoints & Rundenzählung

- BLE iBeacon Setup
- Beacon: BLE Scanner
- Backend: Lap Counter Service
- Admin-UI: Team-Verwaltung
- Datenbank-Schema

### Phase 3: Karten & Checkpoint-Positionierung

- Leaflet Integration
- Checkpoint-Editor (Drag-Drop)
- Track Geometry System
- Live-View: Beacons auf Karte

### Phase 4: BLE RSSI Triangulation

- Gateway: BLE RSSI Scanning
- Backend: Trilateration Algorithmus
- Position-Interpolation
- Live-Karte: Bewegte Positionen

### Phase 5: IMU & Safety

- IMU Integration (MPU6050)
- Crash-Detection Algorithmus
- Alarm-System (Frontend + Display)
- Piezo-Warnungen

### Phase 6: Display-System

- Display-Firmware (LoRa + WS2812)
- Backend: Display Controller
- Admin-UI: Ampel & Restzeit

### Phase 7: Advanced Features

- Team-Login & Auswertung
- Multi-Rennen Statistiken
- SD-Karte Auto-Backup
- Cloud-Streaming (optional)

### Phase 8: GPS/UWB Vorbereitung

- Modulare Architektur für GPS/UWB
- Position-Tracker Multi-Source Support
- Config-basierte Auswahl

## Technische Details

### LoRa Protokoll

**Beacon → Gateway:**

```c
struct BeaconMessage {
  uint8_t beaconID;
  uint8_t messageType;  // 0x01=Telemetry, 0x02=Checkpoint, 0x03=Crash
  uint32_t timestamp;
  int8_t rssi_gateway[4];
  uint8_t battery_percent;
  int16_t accel_x, accel_y, accel_z;
  uint8_t last_checkpoint_id;
  uint32_t checkpoint_timestamp;
};
```

- Frequenz: 868MHz (EU)
- Spreading Factor: 7-9
- Bandwidth: 125kHz
- Rate: 1 Nachricht/s pro Beacon

### BLE iBeacon Format

```
UUID: Checkpoint-Typ (START, FINISH, BOX, SECTOR1)
Major: Checkpoint-ID (1, 2, 3...)
Minor: 0 (reserviert)
TX Power: -59 (kalibriert für 2-3m)
```

### RSSI → Distanz (Log-Distance Path Loss)

```python
def rssi_to_distance(rssi, tx_power=-59, n=2.5):
    return 10 ** ((tx_power - rssi) / (10 * n))
```

- n=2.5 für Outdoor
- Kalibrierung vor Ort nötig

### Batterie-Management

- Deep-Sleep: 10-50µA
- LoRa TX: ~120mA für 100ms → ~3.3mAh/s bei 1Hz
- BLE Scan: ~50mA kontinuierlich
- Optimierung: BLE Scan nur 200ms alle 500ms
- Erwartung: 1500mAh → 10-12h

## Kosten-Kalkulation

### Prototyp (3 Beacons): ~200€

- ESP32 + LoRa (Beacons): 3x 9€ = 27€
- IMU: 3x 2€ = 6€
- BLE Checkpoints: 5x 3€ = 15€
- LoRa Gateways: 3x 15€ = 45€
- Raspberry Pi 4: 60€
- Display (ESP32+LoRa+WS2812): 20€
- Akkus, Kabel: 30€

### Voll-System (20 Beacons): ~460€

- Prototyp: 200€
- 17 weitere Beacons: 17x 10€ = 170€
- 2 weitere Displays: 2x 20€ = 40€
- Mehr Akkus & Gehäuse: 50€

**Optional:**

- GPS (+100€): 5€ x 20 Beacons
- UWB (+300€): 15€ x 20 Beacons

---

# DETAILLIERTER WORKFLOW (FullBlown)

## Ersteinrichtung (einmalig, ~30 Min)

### 1. Hardware aufbauen

- 3-4 LoRa Gateways an Streckenrand aufstellen (hohe Position, freie Sicht)
- Raspberry Pi starten → WiFi Hotspot "MoRa-LC-Hotspot"
- Admin verbindet Laptop: http://192.168.4.1

### 2. Gateways kalibrieren

- Admin-UI → "Gateways"
- Für jedes Gateway Position setzen (auf Karte klicken oder GPS)
- System zeigt Abdeckungs-Vorschau

### 3. Strecken-Karte erstellen

**Option A:** Satellitenansicht

- Adresse eingeben → OpenStreetMap laden
- Strecken-Umriss zeichnen
- Maßstab kalibrieren

**Option B:** Leere Karte

- Größe eingeben (75x75m)
- Raster-Hintergrund

### 4. BLE Checkpoints einrichten

**Physisch:**

- Xiaomi Mi Beacons an Start/Ziel, Boxengasse, Sektoren platzieren
- Batterie einlegen, einschalten

**Digital:**

- Admin-UI → "Checkpoints"
- "BLE Scan starten" → Gateways scannen 10s
- Für jeden erkannten Beacon:
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Typ zuweisen (Start/Ziel, Sektor, Box)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Position auf Karte klicken
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Speichern

### 5. Mofa-Beacons vorbereiten

**Firmware flashen (einmalig):**

- PlatformIO → env:beacon
- config.h: BEACON_ID setzen (1-99)
- Upload → Piezo piept 3x

**Vor Rennen:**

- Admin-UI → "Beacons"
- Status-Check: Online, Batterie >20%

## Rennen-Vorbereitung (~10 Min)

### 1. Neues Rennen erstellen

- Admin-UI → "Rennen" → "Neues Rennen"
- Name, Strecke, Datum, Modus (Zeit/Runden), Dauer eingeben

### 2. Teams & Beacons zuordnen

- Team hinzufügen: Name, Fahrer, Beacon auswählen, Farbe
- Wiederholen für alle Teams (max 10 gleichzeitig)

### 3. Pre-Race Check

- System prüft: Strecke, Checkpoints, Gateways, Beacons, Display, SD-Karte
- Warnungen beheben (z.B. schwache Batterie)

## Rennen-Start (~3 Min)

### Startprozedur

1. "Start-Sequenz beginnen"
2. **Fahrzeuge positionieren:** Warte bis alle an Start/Ziel erkannt
3. **Countdown:** 10/30s wählen oder individuell

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Display zeigt: ROT → GELB → GRÜN
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Rennen startet, Timer läuft

## Während des Rennens

**Admin-Ansicht:**

- Live-Karte mit allen Beacons
- Rangliste (Runden + Zeiten)
- Event-Log
- Aktionen: Strafe vergeben, Ampel steuern, Crash bestätigen

**Öffentliche Live-View:**

- Separate URL für Zuschauer
- Nur Karte, Rangliste, Zeiten
- Auto-Update via WebSocket

## Rennen-Ende

- Zeit abgelaufen oder Admin beendet
- Finale Rangliste anzeigen
- Daten auf SD-Karte gespeichert
- Export als CSV
- Team-Zugänge per QR-Code generieren

---

# ULTRALIGHT VARIANTE

## Konzept

**Minimalistische Standalone-Lösung für kleine Rennen**

### Hardware (~90€ für 20 Beacons)

**CheapYellow Display (ESP32-2432S028)** - 15€

- 2.4" TFT Touch Display (320x240)
- ESP32 mit BLE Scanner integriert
- SD-Karte Slot
- **Steht an Start/Ziel = IS der Checkpoint!**

**Xiaomi Mi Beacons (Mofas)** - 20x 3€ = 60€

- Nur iBeacon Broadcasting
- CR2032 Batterie (Monate Laufzeit)
- Mit Aufklebern nummeriert (#1, #2, #3...)

**Powerbank** - 15€

**Keine separaten Checkpoints, keine Gateways, kein Laptop nötig!**

### Features

✅ Rundenzählung (Display erkennt BLE Beacons)

✅ Rundenzeiten pro Team

✅ Rangliste

✅ Team-Management per Touch

✅ SD-Karte Logging

✅ Beacon-Zuordnung: Aufkleber + RSSI-Näherung

❌ Keine Karte/Position zwischen Checkpoints

❌ Keine Live-View für Zuschauer

❌ Kein Cloud-Streaming

❌ Keine Crash-Detection

### Software

```
/firmware/
  /ultralight/
    /src/
      main.cpp              # CheapYellow Firmware
      ui/
        screen_home.cpp     # Hauptmenü
        screen_teams.cpp    # Team-Management
        screen_race.cpp     # Rennen läuft
        screen_results.cpp  # Ergebnisse
      ble_scanner.cpp       # BLE Scanning
      lap_counter.cpp       # Rundenzählung
      data_logger.cpp       # SD-Karte
      display_driver.cpp    # TFT (TFT_eSPI)
    platformio.ini
```

## UltraLight Workflow

### Setup (einmalig, ~5 Min)

1. **Display flashen:**

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - PlatformIO → ultralight environment
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Upload

2. **Display an Start/Ziel aufstellen:**

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Powerbank anschließen
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Einschalten

3. **Beacons vorbereiten:**

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Xiaomi Mi Beacons auspacken
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Mit Aufklebern nummerieren (#1, #2, #3...)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - Batterie einlegen
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - An Mofas befestigen
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                - **Fertig!** (kein Flashen nötig)

### Rennen vorbereiten (auf Display, ~5 Min)

**Hauptmenü:**

```
┌──────────────────────────┐
│   MoRa-LC UltraLight     │
├──────────────────────────┤
│   [Neues Rennen]         │
│   [Teams verwalten]      │
│   [Ergebnisse anzeigen]  │
└──────────────────────────┘
```

**Teams verwalten:**

```
1. [+ Neues Team] → Name eingeben
2. [Beacon zuordnen]
3. Zwei Methoden:

   Methode A - Einzeln aktivieren:
   - Alle Beacons ausgeschaltet lassen
   - Nur Beacon #1 einschalten
   - Display zeigt einen Beacon → Zuordnen
   - Beacon #1 ausschalten, #2 einschalten → Zuordnen
   - usw.

   Methode B - RSSI-Näherung:
   - Alle Beacons eingeschaltet
   - "Beacon zuordnen" antippen
   - Mofa mit Beacon direkt vor Display halten (<1m)
   - Display wählt automatisch nächsten (stärkster RSSI)
   - Bestätigen → Zugeordnet
   
4. Wiederholen für alle Teams
```

**Rennen starten:**

```
┌──────────────────────────┐
│ Rennen starten           │
├──────────────────────────┤
│ Name: [Lauf 1]           │
│ Modus: Zeitrennen        │
│ Dauer: [< 60 Min >]      │
│                          │
│ Teams (8 ausgewählt):    │
│ ☑ Team 1 (Beacon #3)     │
│ ☑ Team 2 (Beacon #7)     │
│ ...                      │
│                          │
│ [Countdown starten]      │
└──────────────────────────┘
```

### Rennen läuft

```
┌──────────────────────────┐
│ ⏱️ 42:15                 │
├──────────────────────────┤
│ 1. Blitz-Mofas    12R    │
│    Letzte: 3:24.5        │
│                          │
│ 2. Speed Demons   12R    │
│    Letzte: 3:28.1        │
│                          │
│ 3. Turbo Turtles  11R    │
│    Letzte: 3:31.8        │
│                          │
│ (scroll für mehr)        │
│                          │
│ [Pause] [Beenden]        │
└──────────────────────────┘

System:
- BLE Scanner läuft kontinuierlich
- Erkennt Beacons bei Vorbeifahrt (<3m)
- Zählt Runde + speichert Zeit
- Update Display
- Schreibt auf SD-Karte
```

### Nach dem Rennen

```
┌──────────────────────────┐
│ Ergebnisse: "Lauf 1"     │
├──────────────────────────┤
│ 🥇 Blitz-Mofas           │
│    14 Runden             │
│    Beste: 3:18.2         │
│    [Details anzeigen]    │
│                          │
│ 🥈 Speed Demons          │
│    14 Runden             │
│    Beste: 3:22.5         │
│                          │
│ Auf SD: races/lauf1.csv  │
│                          │
│ [Neues Rennen]           │
│ [Hauptmenü]              │
└──────────────────────────┘
```

### Optionale Erweiterungen

- Zweites Display für Zuschauer (BLE Slave)
- Export per WiFi (Hotspot kurz öffnen → CSV Download)
- Strafen-Eingabe (Runden abziehen/Zeit addieren)

---

# GEMEINSAME CODE-BASIS

Beide Varianten teilen Module:

```
/lib/
  /BLEScanner/         # BLE Scanning Logik
  /LapCounter/         # Rundenzählung Algorithmus
  /DataLogger/         # SD-Karte Logging
  /LoRaComm/           # Nur FullBlown
  /IMUHandler/         # Nur FullBlown
  /PositionTracker/    # Nur FullBlown
```

---

# NÄCHSTE SCHRITTE

## Start mit UltraLight (einfacher)

1. CheapYellow Display bestellen + Xiaomi Mi Beacons (3x zum Testen)
2. PlatformIO UltraLight-Firmware entwickeln
3. BLE Scanner + Lap Counter + UI implementieren
4. Field-Test auf Strecke

## Parallel: FullBlown Basis

1. ESP32 + LoRa Module bestellen
2. PlatformIO Multi-Environment Setup
3. LoRa Protokoll + Beacon-Firmware
4. Gateway + Python Backend Basis

## Nach Field-Tests

- Refinement basierend auf Erfahrungen
- Entscheidung: Welche Variante zuerst fertigstellen?
- Optional: GPS/UWB Integration planen

### To-dos

- [ ] Repository initialisieren: Ordnerstruktur, Git, README, .gitignore für beide Varianten
- [ ] PlatformIO Multi-Environment Setup (beacon, gateway, display, ultralight) mit shared libraries
- [ ] UltraLight: CheapYellow Basis-Firmware (TFT Init, Touch, Hauptmenü)
- [ ] UltraLight: BLE Scanner implementieren (Beacon-Erkennung, RSSI)
- [ ] UltraLight: UI Screens (Teams verwalten, Beacon-Zuordnung per RSSI)
- [ ] UltraLight: Lap Counter (Rundenzählung, Zeiten, Rangliste)
- [ ] UltraLight: SD-Karte Logging (CSV Export)
- [ ] FullBlown: LoRa Protokoll definieren (Message Structs, Dokumentation)
- [ ] FullBlown: Beacon-Firmware Basis (LoRa Telemetrie senden)
- [ ] FullBlown: Beacon BLE Scanner (Checkpoint-Erkennung)
- [ ] FullBlown: Beacon IMU Integration (MPU6050, Crash-Detection)
- [ ] FullBlown: Gateway-Firmware (LoRa empfangen, MQTT Bridge, BLE RSSI)
- [ ] FullBlown: Python Backend Basis (FastAPI, MQTT Client, Datenbank)
- [ ] FullBlown: Lap Counter Service (Checkpoint-basierte Rundenzählung)
- [ ] FullBlown: Position Tracker (BLE RSSI Trilateration)
- [ ] FullBlown: React Frontend Setup (Vite, TailwindCSS, Leaflet)
- [ ] FullBlown: Admin-UI (Team-Management, Strecken-Setup, Checkpoint-Editor)
- [ ] FullBlown: Live-View (Karte, Rangliste, WebSocket)
- [ ] FullBlown: Display-Firmware (LoRa Empfang, WS2812, Ampel/Countdown)
- [ ] Field-Test: Beide Systeme auf echter Strecke testen und optimieren