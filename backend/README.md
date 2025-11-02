# Backend

Python Backend für das FullBlown MoRa-LC System.

## Stack

- **FastAPI** - REST API + WebSocket
- **SQLAlchemy** - ORM
- **SQLite** - Datenbank
- **Paho-MQTT** - Hardware-Kommunikation
- **NumPy/SciPy** - RSSI-Triangulation
- **Pandas** - Datenauswertung

## Setup

```bash
# Virtual Environment
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate

# Dependencies installieren
pip install -r requirements.txt

# Development Server starten
python -m app.main
```

Server läuft auf `http://localhost:8000`

API Docs: `http://localhost:8000/docs`

## Struktur

```
/app/
  main.py                 # FastAPI App Entry
  /api/
    teams.py              # Team-Management Endpoints
    races.py              # Rennen-Steuerung
    telemetry.py          # Live-Daten WebSocket
    admin.py              # Admin-Funktionen
    track.py              # Strecken-Setup
  /services/
    lora_handler.py       # LoRa Message Processing
    position_tracker.py   # Position aus RSSI + Checkpoints
    ble_triangulation.py  # RSSI → Position
    lap_counter.py        # Rundenzählung
    crash_detector.py     # Sturz-Detection
    penalty_system.py     # Strafen-System
    data_logger.py        # SD-Karte Logging
    display_controller.py # Display-Steuerung
  /models/
    database.py           # SQLAlchemy Models
    schemas.py            # Pydantic Schemas
  /utils/
    mqtt_client.py        # MQTT Setup
    track_geometry.py     # Geometrie-Berechnungen
```

## Configuration

`config.yaml`:

```yaml
mqtt:
  broker: localhost
  port: 1883
  topics:
    lora_rx: "mora/lora/rx"
    lora_tx: "mora/lora/tx"

database:
  url: "sqlite:///./data/mora.db"

lora:
  frequency: 868
  spreading_factor: 7

track:
  size_x: 75
  size_y: 75
```

## API Endpoints

### Teams

```
GET    /api/teams              # Liste aller Teams
POST   /api/teams              # Neues Team erstellen
GET    /api/teams/{id}         # Team Details
PUT    /api/teams/{id}         # Team aktualisieren
DELETE /api/teams/{id}         # Team löschen
POST   /api/teams/{id}/beacon  # Beacon zuordnen
```

### Races

```
GET    /api/races              # Liste aller Rennen
POST   /api/races              # Neues Rennen erstellen
GET    /api/races/{id}         # Rennen Details
PUT    /api/races/{id}/start   # Rennen starten
PUT    /api/races/{id}/pause   # Rennen pausieren
PUT    /api/races/{id}/stop    # Rennen beenden
```

### Telemetry (WebSocket)

```
WS     /api/telemetry/live     # Live-Daten Stream
```

### Admin

```
GET    /api/admin/beacons      # Beacon-Status
GET    /api/admin/gateways     # Gateway-Status
POST   /api/admin/display      # Display-Kommando senden
```

## Development

### Database Migrations

```bash
# Neue Migration erstellen
alembic revision --autogenerate -m "Add teams table"

# Migration anwenden
alembic upgrade head

# Rollback
alembic downgrade -1
```

### Testing

```bash
# Unit Tests
pytest

# Mit Coverage
pytest --cov=app tests/

# Spezifischer Test
pytest tests/test_lap_counter.py -v
```

### Debug Mode

```bash
# Mit Auto-Reload
uvicorn app.main:app --reload --log-level debug
```

## MQTT Topics

**Von Hardware (Gateways):**
- `mora/lora/rx` - LoRa Nachrichten von Beacons
- `mora/beacon/{id}/telemetry` - Beacon-Telemetrie
- `mora/checkpoint/{id}/event` - Checkpoint-Events

**An Hardware:**
- `mora/lora/tx` - LoRa Kommandos
- `mora/display/{id}/command` - Display-Kommandos

## Services

### LoRa Handler

Verarbeitet eingehende LoRa-Nachrichten von Beacons:
- Telemetrie (Batterie, RSSI, IMU)
- Checkpoint-Events
- Crash-Alerts

### Position Tracker

Berechnet Position aus:
1. BLE RSSI von 3+ Gateways (Trilateration)
2. Checkpoint-Position (bekannt)
3. Interpolation entlang Strecke

### Lap Counter

Rundenzählung-Logik:
- Start/Ziel Checkpoint zwingend
- Validierung: Alle Pflicht-Checkpoints passiert
- Rundenzeit-Berechnung

### Crash Detector

Sturz-Erkennung aus IMU-Daten:
- Beschleunigung >2.5G
- Plötzliche Orientierungsänderung
- Alert an Frontend & Display

## Troubleshooting

**MQTT Connection failed:**
- Mosquitto Broker läuft? `sudo systemctl status mosquitto`
- Config korrekt? `config.yaml`

**Database locked:**
- SQLite nur ein Writer - zu viele parallele Zugriffe
- Lösung: PostgreSQL für Production

**WebSocket disconnects:**
- Timeout zu kurz - in `config.yaml` erhöhen
- Nginx Proxy? WebSocket Support aktivieren



