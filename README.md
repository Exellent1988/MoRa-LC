# MoRa-LC: Mofarennen Lap Counter & Timing System

Ein modulares Timing-System für Mofarennen mit zwei Varianten für unterschiedliche Anforderungen.

## 🏁 Projekt-Übersicht

### Variante 1: FullBlown (~460€ für 20 Beacons)
- **Technologie:** Raspberry Pi + Web-App + LoRa + BLE RSSI Triangulation
- **Features:** Live-Tracking auf Karte, Cloud-Streaming, IMU-Sturzerkennung
- **Zielgruppe:** Professionelle Events mit bis zu 20 Teams
- **Abdeckung:** 75x75m Strecke

### Variante 2: UltraLight (~90€ für 20 Beacons)
- **Technologie:** CheapYellow Touch-Display + BLE Beacons
- **Features:** Rundenzählung + Zeiten per Touch-UI
- **Zielgruppe:** Kleine lokale Rennen, kein Laptop nötig
- **Abdeckung:** Start/Ziel Checkpoint

## 📁 Projekt-Struktur

```
MoRa-LC/
├── firmware/              # ESP32 Firmware (PlatformIO)
│   ├── lib/              # Shared Libraries
│   │   ├── BLEScanner/   # BLE Scanning (beide Varianten)
│   │   ├── LapCounter/   # Rundenzählung (beide)
│   │   ├── DataLogger/   # SD-Karte Logging (beide)
│   │   ├── LoRaComm/     # LoRa (nur FullBlown)
│   │   ├── IMUHandler/   # IMU (nur FullBlown)
│   │   └── PositionTracker/ # Position (nur FullBlown)
│   ├── beacon/           # Mofa-Beacon Firmware (FullBlown)
│   ├── gateway/          # Gateway Firmware (FullBlown)
│   ├── checkpoint/       # Smart Checkpoint (FullBlown, optional)
│   ├── display/          # LED Display Firmware (FullBlown)
│   └── ultralight/       # CheapYellow Firmware (UltraLight)
│
├── backend/              # Python Backend (FullBlown)
│   ├── app/
│   │   ├── api/         # REST API Endpoints
│   │   ├── services/    # Business Logic
│   │   ├── models/      # Datenbank Models
│   │   └── utils/       # Hilfsfunktionen
│   ├── requirements.txt
│   └── config.yaml
│
├── frontend/             # React Frontend (FullBlown)
│   ├── src/
│   │   ├── components/  # UI Komponenten
│   │   ├── views/       # Seiten/Views
│   │   ├── services/    # API & WebSocket
│   │   └── utils/       # Hilfsfunktionen
│   └── package.json
│
├── docs/                 # Dokumentation
│   ├── hardware/        # Hardware-Setup Anleitungen
│   ├── api/             # API Dokumentation
│   └── workflows/       # Workflow-Guides
│
└── hardware/            # Hardware-Infos, Schaltpläne, BOMs
```

## 🚀 Schnellstart

### UltraLight (Einfacher Start)

**Hardware benötigt:**
- CheapYellow Display (ESP32-2432S028) - ~15€
- 3x Xiaomi Mi Beacon für Tests - ~9€
- Powerbank - ~15€

**Software-Setup:**
```bash
# PlatformIO installieren (VS Code Extension)
# Dann:
cd firmware/ultralight
pio run -t upload
```

Siehe `docs/ultralight-quickstart.md` für Details.

### FullBlown (Komplettes System)

**Hardware benötigt (Prototyp):**
- 3x ESP32 + LoRa Module (Beacons)
- 3x ESP32 + LoRa Module (Gateways)  
- Raspberry Pi 4
- 5x BLE Beacons (Checkpoints)

**Software-Setup:**
```bash
# 1. Firmware flashen
cd firmware/beacon
pio run -t upload

# 2. Backend starten
cd backend
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
python -m app.main

# 3. Frontend starten
cd frontend
npm install
npm run dev
```

Siehe `docs/fullblown-quickstart.md` für Details.

## 📖 Dokumentation

- **[Hardware-Setup](docs/hardware/)** - Komponenten, Verkabelung, Montage
- **[Firmware-Entwicklung](firmware/README.md)** - PlatformIO, Libraries
- **[Backend-API](docs/api/)** - REST Endpoints, WebSocket
- **[Frontend-Entwicklung](frontend/README.md)** - React, UI Komponenten
- **[Workflows](docs/workflows/)** - Rennen-Vorbereitung, Durchführung

## 🛠️ Entwicklung

### Voraussetzungen

- **PlatformIO** (VS Code Extension oder CLI)
- **Python 3.9+** (für Backend)
- **Node.js 18+** (für Frontend)
- **Git**

### Entwicklungs-Setup

```bash
# Repository klonen
git clone <repo-url>
cd MoRa-LC

# Python venv erstellen
cd backend
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# Node modules installieren
cd ../frontend
npm install

# PlatformIO Dependencies
# Werden automatisch bei erstem Build geladen
```

### Coding-Standards

- **Commit-Nachrichten:** Deutsch, Format `type(scope): subject`
  - Types: `feat` (minor), `fix`/`refactor`/`perf`/`docs`/`style`/`test` (patch)
  - Breaking Changes: `BREAKING CHANGE:` für major bump
- **Python:** PEP 8, Type Hints
- **C++:** Google Style Guide (angepasst)
- **JavaScript:** ESLint + Prettier

## 🔧 Hardware-Komponenten

### FullBlown System

| Komponente | Anzahl (Prototyp) | Preis/Einheit | Gesamt |
|------------|-------------------|---------------|---------|
| ESP32 DevKit | 6 | 4€ | 24€ |
| LoRa RFM95W | 6 | 5€ | 30€ |
| IMU MPU6050 | 3 | 2€ | 6€ |
| BLE Beacons (Checkpoint) | 5 | 3€ | 15€ |
| Raspberry Pi 4 | 1 | 60€ | 60€ |
| WS2812 LED Panel | 1 | 10€ | 10€ |
| Akkus, Kabel, Gehäuse | - | - | 55€ |
| **Gesamt Prototyp** | | | **~200€** |

### UltraLight System

| Komponente | Anzahl | Preis/Einheit | Gesamt |
|------------|--------|---------------|---------|
| CheapYellow Display | 1 | 15€ | 15€ |
| Xiaomi Mi Beacon | 20 | 3€ | 60€ |
| Powerbank | 1 | 15€ | 15€ |
| **Gesamt** | | | **~90€** |

## 📊 Features

### FullBlown
✅ Live-Position auf Karte (BLE RSSI Triangulation)  
✅ Rundenzählung & Zeiten  
✅ IMU-Sturzerkennung  
✅ Web-App für Admin & Zuschauer  
✅ WS2812 LED-Display (Ampel, Countdown)  
✅ Cloud-Streaming (optional)  
✅ Multi-Rennen Auswertung  
✅ Team-Login & Detailauswertung  
✅ GPS/UWB erweiterbar  

### UltraLight
✅ Rundenzählung & Zeiten  
✅ Touch-UI auf Display  
✅ Team-Management  
✅ SD-Karte Logging
✅ RSSI-basierte Beacon-Zuordnung  
❌ Keine Karte/Live-Position  
❌ Keine IMU-Sensoren  
❌ Keine Cloud-Features  

## 🤝 Contributing

Dieses Projekt ist für persönliche/Community-Nutzung. Contributions sind willkommen!

1. Fork erstellen
2. Feature-Branch erstellen (`git checkout -b feat/amazing-feature`)
3. Änderungen committen (`git commit -m 'feat: Add amazing feature'`)
4. Push zum Branch (`git push origin feat/amazing-feature`)
5. Pull Request öffnen

## 📝 Lizenz

[MIT License](LICENSE) - Frei für private und kommerzielle Nutzung.

## 🙏 Acknowledgments

- **PlatformIO** - ESP32 Development Platform
- **FastAPI** - Python Backend Framework
- **React** - Frontend Framework
- **Leaflet** - Karten-Integration
- **LoRa Community** - Protokoll-Dokumentation

## 📧 Kontakt

Bei Fragen oder Problemen: [Issue erstellen](../../issues)

---

**Status:** 🚧 In Entwicklung

**Aktueller Fortschritt:**
- [x] Projekt-Setup & Planung
- [x] **UltraLight v3** - ✅ **Bereit zum Testen!**
  - [x] Clean Architecture Rewrite mit LVGL8
  - [x] Vollständige UI mit allen Screens (Home, Teams, Race Setup/Running/Results, Settings)
  - [x] Dialog-System für Bestätigungen
  - [x] Konsistentes Font- und Theme-System
  - [x] Persistenz (Teams bleiben nach Neustart erhalten)
  - [x] BLE Beacon Scanning & RSSI-basierte Rundenzählung
  - [x] SD-Karte Logging mit SdFat (CSV Export)
  - [x] Navigation zwischen allen Screens
  - [x] Settings: Teams speichern, BLE Settings, Reset
  - Siehe [UltraLight Build Guide](docs/ultralight-build.md)
- [x] **UltraLight v2** - ✅ (Legacy, nicht mehr aktiv entwickelt)
- [ ] FullBlown Basis-System
- [ ] Field-Tests
- [ ] Finale Features

Siehe [TODOs](.cursor/plans/) für Details.
