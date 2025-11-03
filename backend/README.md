# Backend

Python Backend fuer das FullBlown MoRa-LC System.

## Aktueller Stand (November 2025)

- [x] FastAPI Grundgeruest mit automatischer Tabellenerstellung
- [x] SQLAlchemy-Modelle fuer `Team`, `Race` und `RaceTeam`
- [x] REST-Endpoints fuer Team- und Rennverwaltung (inkl. Statuswechsel)
- [x] Konfiguration via `.env` oder `config.yaml`
- [x] Health-Check Endpoint (`/health`)
- [ ] MQTT-, Telemetrie- und Service-Layer folgen als naechstes

## Setup

```bash
# Virtual Environment (Pfad an bestehendes venv anpassen)
python -m venv venv
source venv/bin/activate

# Dependencies installieren
pip install -r requirements.txt

# Datenbankordner vorbereiten (optional)
mkdir -p data

# Server starten
uvicorn app.main:app --reload --log-level info
```

Server laeuft auf `http://localhost:8000`

- API Docs: `http://localhost:8000/docs`
- ReDoc: `http://localhost:8000/redoc`

## Projektstruktur

```
/backend
  config.yaml            # Default-Konfiguration
  requirements.txt       # Python Dependencies
  /app
    __init__.py
    main.py              # FastAPI App Factory
    /api
      __init__.py        # Router Aggregation
      teams.py           # Team-Endpunkte
      races.py           # Renn-Endpunkte
    /core
      config.py          # Settings + YAML Loader
    /db
      base.py            # SQLAlchemy Base
      session.py         # Engine + Session Handling
    /models
      __init__.py
      team.py            # Team-Modelle
      race.py            # Race + RaceTeam Modelle
    /schemas
      __init__.py
      team.py            # Pydantic Team Schemas
      race.py            # Pydantic Race Schemas
```

## Konfiguration

`config.yaml` kann pro Deployment angepasst werden und ueberschreibt Defaults aus den Environment-Variablen:

```yaml
general:
  app_name: "MoRa-LC Backend"
  debug: false

mqtt:
  broker: localhost
  port: 1883
  topics:
    lora_rx: "mora/lora/rx"
    lora_tx: "mora/lora/tx"

database:
  url: "sqlite:///./data/mora.db"

track:
  size_x: 75
  size_y: 75
```

## REST API

### Teams (`/api/teams`)

- `GET /` ? Liste aller Teams
- `POST /` ? Neues Team anlegen
- `GET /{id}` ? Team-Details
- `PUT /{id}` ? Team aktualisieren
- `DELETE /{id}` ? Team loeschen
- `POST /{id}/beacon` ? Beacon-MAC zuordnen
- `GET /summaries` ? Kompakte Teamliste (ID + Name)

### Races (`/api/races`)

- `GET /` ? Alle Rennen
- `POST /` ? Neues Rennen (inkl. optionaler Team-ID-Liste)
- `GET /{id}` ? Renn-Details
- `PUT /{id}` ? Allgemeine Updates (Name, Dauer, Status)
- `PUT /{id}/start` ? Rennen starten/fortsetzen
- `PUT /{id}/pause` ? Rennen pausieren
- `PUT /{id}/stop` ? Rennen beenden

Alle Rennen liefern verknuepfte Teams als `TeamSummary` zurueck.

## Naechste Schritte

- MQTT-Client und LoRa-Handler anbinden
- Service-Layer (Lap Counter, Position Tracker, Crash Detection) implementieren
- WebSocket-Streaming der Telemetriedaten
- Alembic fuer Datenbank-Migrationen konfigurieren
- Automatisierte Tests (pytest) aufsetzen

## Troubleshooting

- `sqlite3.OperationalError: unable to open database file` ? sicherstellen, dass `data/` existiert und Schreibrechte vorhanden sind.
- 404 bei `/api/...` ? pruefen, ob Server mit `uvicorn app.main:app` gestartet wurde.
- Enum-Fehler bei Migrationen ? bei Wechsel auf PostgreSQL `alembic`-Konfiguration anpassen (SQLAlchemy Enum -> native Enum).
