# UltraLight Webserver - Implementierungsplan

**Ziel:** WLAN Hotspot vom ESP32 mit Web-Interface für Verwaltung, Live-Daten und OTA-Updates

**Status:** Planung  
**Priorität:** Optional/Erweiterung

## Übersicht

Der ESP32 erstellt einen eigenen WLAN Hotspot. Benutzer können sich mit Smartphone/Tablet/Laptop verbinden und auf ein Web-Interface zugreifen.

### Features

1. **Geschützter Admin-Bereich** (Login)
2. **Live-Rennen Anzeige** (Rangliste, Zeiten)
3. **Team-Verwaltung** (Web-UI für Teams & Beacons)
4. **Rennen-Verwaltung** (Start/Stop/Pause per Web)
5. **Ergebnisse-Download** (CSV-Dateien)
6. **OTA Updates** (Firmware-Update über Web-Interface)
7. **System-Info** (SD-Karte Status, Speicher, Version)

## Technische Architektur

### WLAN Hotspot

```
ESP32 Access Point (AP Mode)
- SSID: "MoRa-LC-<DeviceID>" (z.B. "MoRa-LC-1234")
- Passwort: Konfigurierbar (Standard: "mora-lc-2025")
- IP: 192.168.4.1
- DHCP: Aktiviert (192.168.4.2 - 192.168.4.10)
```

### Webserver Stack

- **ESPAsyncWebServer** (async, nicht-blockierend)
- **SPIFFS** oder **LittleFS** für Web-Assets (HTML, CSS, JS)
- **ArduinoJson** für API Responses
- **ESP32 OTA** für Firmware-Updates

### API Endpoints

#### Öffentliche Endpoints (kein Login)

```
GET  /                          → Hauptseite (Live-View oder Login)
GET  /api/race/status           → Aktueller Rennen-Status
GET  /api/race/leaderboard      → Live-Rangliste (JSON)
GET  /api/race/teams            → Teams-Liste (JSON)
GET  /api/race/laps/:teamId     → Rundenzeiten für Team (JSON)
```

#### Geschützte Endpoints (Admin-Login nötig)

```
POST /api/auth/login            → Login (Session Cookie)
POST /api/auth/logout           → Logout

GET  /admin                     → Admin-Dashboard
GET  /admin/teams               → Team-Verwaltung
GET  /admin/teams/:id           → Team bearbeiten
POST /api/admin/teams           → Neues Team erstellen
PUT  /api/admin/teams/:id       → Team aktualisieren
DELETE /api/admin/teams/:id     → Team löschen

POST /api/admin/race/start      → Rennen starten
POST /api/admin/race/pause      → Rennen pausieren
POST /api/admin/race/resume     → Rennen fortsetzen
POST /api/admin/race/stop       → Rennen beenden

GET  /api/admin/results          → Liste aller Rennen (JSON)
GET  /api/admin/results/:id      → Rennen-Details (JSON)
GET  /api/admin/results/:id/download → CSV-Download

GET  /api/admin/system/info      → System-Informationen
GET  /api/admin/system/sd       → SD-Karte Status
POST /api/admin/system/sd/format → SD-Karte formatieren
GET  /api/admin/system/logs      → Serial-Logs (letzte 100 Zeilen)

GET  /ota                       → OTA Update-Seite
POST /api/admin/ota/upload      → Firmware hochladen
POST /api/admin/ota/update      → Update starten
```

### Authentifizierung

**Einfache Session-basierte Auth:**
- Login-Seite: Benutzername + Passwort
- Default: `admin` / `mora-lc-2025` (erste Änderung nach erstem Login!)
- Session-Cookie: `mora_lc_session` (Gültigkeit: 24h)
- Passwort kann später per Einstellungen geändert werden (gespeichert in NVS)

**Erweiterte Optionen (später):**
- Multi-User Support
- Token-basierte API (für externe Clients)
- HTTPS (selbst-signiertes Zertifikat)

## Web-Interface Design

### 1. Live-View (Öffentlich, kein Login)

```
┌─────────────────────────────────────┐
│  MoRa-LC Live View                  │
├─────────────────────────────────────┤
│  ⏱ Zeit: 42:15                      │
│                                     │
│  🥇 1. Blitz-Mofas     12 Runden   │
│      Letzte: 3:24.5                │
│                                     │
│  🥈 2. Speed Demons    12 Runden   │
│      Letzte: 3:28.1                │
│                                     │
│  🥉 3. Thunder Riders  11 Runden   │
│      Letzte: 3:31.2                │
│                                     │
│  [Auto-Refresh: 1s]                │
└─────────────────────────────────────┘
```

**Features:**
- Auto-Refresh (WebSocket oder Polling alle 1s)
- Responsive Design (Mobile & Desktop)
- Minimalistisches Design
- Keine Navigation (nur Live-Daten)

### 2. Admin-Dashboard (Login erforderlich)

```
┌─────────────────────────────────────┐
│  Admin Dashboard            [Logout]│
├─────────────────────────────────────┤
│  Status:                            │
│  ⚡ Rennen läuft                    │
│  📊 5 Teams aktiv                   │
│  💾 SD-Karte: OK (2.5 GB frei)      │
│                                     │
│  Schnellzugriff:                    │
│  [Rennen starten]                   │
│  [Rennen pausieren]                 │
│  [Team verwalten]                   │
│  [Ergebnisse]                       │
│                                     │
│  System:                            │
│  📡 WLAN: MoRa-LC-1234              │
│  🔋 RAM: 45 KB frei                 │
│  ⏱ Uptime: 2h 15m                   │
└─────────────────────────────────────┘
```

### 3. Team-Verwaltung

```
┌─────────────────────────────────────┐
│  Teams verwalten            [+ Neu] │
├─────────────────────────────────────┤
│  1. Blitz-Mofas                     │
│     Beacon: AA:BB:CC:DD:EE:FF        │
│     [Bearbeiten] [Löschen]           │
│                                     │
│  2. Speed Demons                    │
│     Beacon: 11:22:33:44:55:66        │
│     [Bearbeiten] [Löschen]           │
│                                     │
│  ...                                │
└─────────────────────────────────────┘
```

### 4. Ergebnisse-Übersicht

```
┌─────────────────────────────────────┐
│  Ergebnisse                         │
├─────────────────────────────────────┤
│  📁 Rennen 2025-11-15_14-30         │
│     Dauer: 60 Min | Teams: 5        │
│     [Details] [Download CSV]        │
│                                     │
│  📁 Rennen 2025-11-15_16-00          │
│     Dauer: 45 Min | Teams: 3        │
│     [Details] [Download CSV]        │
│                                     │
└─────────────────────────────────────┘
```

### 5. OTA Update-Seite

```
┌─────────────────────────────────────┐
│  Firmware Update                    │
├─────────────────────────────────────┤
│  Aktuelle Version: 0.1.0             │
│                                     │
│  Firmware-Datei auswählen:          │
│  [Durchsuchen...] .bin              │
│                                     │
│  ⚠️ WARNUNG:                        │
│  Update kann einige Minuten dauern! │
│  Bitte nicht ausschalten!           │
│                                     │
│  [Update starten]                   │
└─────────────────────────────────────┘
```

## Implementierungsschritte

### Phase 1: Basis Setup

1. **WLAN Hotspot konfigurieren**
   - ESP32 in AP-Mode
   - DHCP Server
   - SSID/Passwort konfigurierbar

2. **ESPAsyncWebServer einrichten**
   - Server auf Port 80
   - Basis-Routing
   - Static File Serving (SPIFFS/LittleFS)

3. **Web-Assets vorbereiten**
   - HTML/CSS/JS für Live-View
   - Responsive Design
   - Minimal JavaScript (Vanilla oder Micro-Framework)

### Phase 2: Öffentliche API

1. **Status-Endpoints**
   - `/api/race/status` (JSON)
   - `/api/race/leaderboard` (JSON)
   - Auto-Refresh (Polling oder WebSocket)

2. **Live-View Seite**
   - HTML-Seite mit Auto-Refresh
   - JSON-Parsing
   - Responsive Layout

### Phase 3: Authentifizierung

1. **Session-Management**
   - Login-Endpoint
   - Cookie-basierte Sessions
   - Middleware für geschützte Routes

2. **Login-Seite**
   - HTML-Form
   - POST zu `/api/auth/login`
   - Redirect zu `/admin`

### Phase 4: Admin-Bereich

1. **Dashboard**
   - Übersichtsseite
   - Status-Anzeige
   - Schnellzugriff-Buttons

2. **Team-Verwaltung**
   - CRUD-Operationen via API
   - Web-UI für Teams
   - Beacon-Zuordnung (RSSI-Methode per WebSocket?)

3. **Rennen-Steuerung**
   - Start/Stop/Pause per API
   - Live-Status-Anzeige

### Phase 5: Ergebnisse & Download

1. **Ergebnisse-Liste**
   - Dateien auf SD-Karte scannen
   - JSON-API für Liste
   - Web-UI für Übersicht

2. **CSV-Download**
   - Datei von SD-Karte lesen
   - HTTP-Response mit CSV
   - Content-Disposition Header

### Phase 6: OTA Updates

1. **OTA-Server Setup**
   - ESP32 OTA Library
   - Upload-Endpoint
   - Progress-Feedback (WebSocket?)

2. **Update-UI**
   - Datei-Upload (multipart/form-data)
   - Progress-Bar
   - Status-Updates

### Phase 7: Erweiterte Features

1. **System-Info**
   - RAM, Flash, SD-Status
   - Log-Viewer (Serial Output)

2. **Einstellungen**
   - WLAN SSID/Passwort ändern
   - Admin-Passwort ändern
   - RSSI-Thresholds anpassen

## Technische Details

### Dependencies

```ini
lib_deps = 
    esphome/ESPAsyncWebServer @ ^3.0.0
    bblanchon/ArduinoJson @ ^6.21.0
    me-no-dev/AsyncTCP @ ^1.1.1
    me-no-dev/ESPAsyncTCP @ ^1.2.3
    bblanchon/ArduinoJson @ ^6.21.0
```

### SPIFFS/LittleFS Setup

```bash
# Web-Assets in /data/ Verzeichnis
# Upload via PlatformIO:
pio run -t uploadfs
```

**Verzeichnis-Struktur:**
```
/data/
  index.html          # Live-View
  admin.html          # Admin-Login
  admin-dashboard.html
  admin-teams.html
  admin-results.html
  admin-ota.html
  css/
    style.css
  js/
    app.js
    admin.js
```

### API Response Format

```json
// GET /api/race/status
{
  "running": true,
  "paused": false,
  "startTime": 1234567890,
  "duration": 3600000,
  "elapsed": 2520000,
  "remaining": 1080000,
  "raceName": "Mofa-Cup Lauf 1",
  "teamCount": 5
}

// GET /api/race/leaderboard
{
  "teams": [
    {
      "id": 1,
      "name": "Blitz-Mofas",
      "laps": 12,
      "bestLap": 192450,
      "lastLap": 204500,
      "totalDuration": 2450000
    },
    ...
  ]
}
```

### Sicherheit

**Basis (Minimum):**
- Passwort-geschützte Admin-Routes
- Session-Timeout (24h)
- Input-Validierung

**Erweitert (später):**
- HTTPS (selbst-signiert)
- CSRF-Protection
- Rate Limiting
- Secure Password Hashing (bcrypt)

## Speicher-Überlegungen

**SPIFFS/LittleFS:**
- Web-Assets: ~50-100 KB (HTML/CSS/JS komprimiert)
- ESP32 Flash: 4 MB typisch, SPIFFS kann ~1.5 MB nutzen

**RAM:**
- Webserver: ~10-15 KB
- Async-Requests: ~2 KB pro Request
- JSON-Buffer: ~5-10 KB

**Empfehlung:**
- Minimale Web-Assets (komprimiertes CSS/JS)
- Keine großen Bilder
- Inline-CSS/JS für kleine Assets

## OTA Update-Strategie

### Option 1: Web-Interface Upload
- Benutzer lädt .bin-Datei hoch
- ESP32 speichert temporär
- Update via OTA Library
- Vorteil: Einfach, keine externe Infra
- Nachteil: Langsam bei großen Dateien

### Option 2: OTA via Internet (später)
- ESP32 verbindet mit WLAN (wenn verfügbar)
- Download von Server
- Vorteil: Schneller, automatisch
- Nachteil: Benötigt Internet & Server

## Testing

### Lokale Tests
1. Hotspot startet korrekt
2. Web-Interface lädt
3. API-Endpoints funktionieren
4. Login/Auth funktioniert
5. OTA Update testen (sicher!)

### Hardware-Tests
- WLAN-Reichweite (10-50m typisch)
- Performance mit mehreren Clients
- SD-Karte Download-Geschwindigkeit
- OTA Update-Stabilität

## Bekannte Einschränkungen

1. **WLAN-Reichweite:** Nur ~10-50m (AP-Mode)
2. **Gleichzeitige Clients:** Max. ~4-8 Clients (ESP32 Limit)
3. **OTA Update:** Kann 2-5 Minuten dauern (kein Neustart währenddessen!)
4. **Speicher:** Web-Assets begrenzt auf ~100 KB

## Zukunftsvisionen

- **Multi-Language Support** (DE/EN)
- **Dark Mode** für Web-Interface
- **Charts/Graphs** für Rundenzeiten-Visualisierung
- **WebSocket** für Echtzeit-Updates (statt Polling)
- **PWA** (Progressive Web App) für Offline-Nutzung
- **Cloud-Backup** (optional, wenn Internet verfügbar)

## Zeitaufwand (Schätzung)

- Phase 1-2 (Basis + Public API): 4-6 Stunden
- Phase 3 (Auth): 2-3 Stunden
- Phase 4 (Admin): 6-8 Stunden
- Phase 5 (Results): 2-3 Stunden
- Phase 6 (OTA): 3-4 Stunden
- Phase 7 (Extras): 2-3 Stunden

**Gesamt:** ~20-30 Stunden Entwicklung

---

**Nächster Schritt:** Implementierung Phase 1 starten (WLAN Hotspot + Basis-Server)