# Firmware

ESP32 Firmware für alle Hardware-Komponenten des MoRa-LC Systems.

## Struktur

### Source Code (`/src/`)

#### UltraLight v3 (`/src/ultralight_v3/`) - **Production-Ready** ✅
Complete Rewrite mit Clean Architecture.

**Hardware:** ESP32-2432S028 (CheapYellow Display)

**Architektur:**
```
ultralight_v3/
├── core/           # System, Config, Memory, Tasks
├── hardware/       # Display, Touch, SD Card, BLE
├── services/       # Beacon, LapCounter, Persistence, DataLogger
└── ui/             # LVGL8 Screens, Navigation, Widgets
```

**Features:**
- LVGL8 UI System (modern, responsive)
- NimBLE BLE Scanning (optimiert für Race Mode)
- Service-basierte Architektur
- SD Card CSV Logging
- Team Management mit Persistence (NVS)
- Automatische Rundenzählung
- Live Leaderboard

**BLE Backend Optionen:**
- NimBLE-Arduino (Standard, beste Performance)
- ESP32 BLE Arduino (Alternative)
- ESP-IDF Native (Optional, experimentell)

#### UltraLight v1 & v2 (Legacy - Deprecated)
Alte Versionen mit TFT_eSPI/LovyanGFX. Nicht mehr in Entwicklung.

### Shared Libraries (`/lib/`)
Legacy Libraries für alte Versionen:
- **BLEScanner** - BLE iBeacon Scanning
- **LapCounter** - Rundenzählung Algorithmus
- **DataLogger** - SD-Karte Logging
- **LoRaComm** - LoRa Kommunikation (FullBlown)

#### FullBlown System

**Beacon (`/beacon/`)**
Mofa-Beacon mit LoRa + BLE + IMU.

**Hardware:** ESP32 + RFM95W + MPU6050

**Features:**
- LoRa Telemetrie (1Hz)
- BLE Scanner (Checkpoint-Erkennung)
- IMU Sturzerkennung
- Piezo Buzzer
- Deep-Sleep Batterie-Optimierung

**Gateway (`/gateway/`)**
LoRa Gateway mit MQTT Bridge.

**Hardware:** ESP32 + RFM95W

**Features:**
- LoRa Empfang
- BLE RSSI Scanning (Triangulation)
- MQTT Bridge (WiFi)
- NTP Zeitstempel

**Checkpoint (`/checkpoint/`)** (Optional)
Smart Checkpoint mit Status-Reporting.

**Hardware:** ESP32 + LoRa (optional)

**Features:**
- iBeacon Broadcasting
- Durchfahrten loggen
- LoRa Status-Meldung

**Display (`/display/`)**
LED Display für Ampel & Countdown.

**Hardware:** ESP32 + RFM95W + WS2812B

**Features:**
- LoRa Kommando-Empfang
- WS2812 Steuerung (FastLED)
- Modi: Ampel, Countdown, Warnung

## Development

### PlatformIO Setup

```bash
# Install PlatformIO
pip install platformio

# Build UltraLight v3:
cd firmware
pio run -e ultralight_v3

# Upload:
pio run -e ultralight_v3 -t upload

# Serial Monitor:
pio device monitor
```

### Environments

Verfügbare PlatformIO Environments in `platformio.ini`:

- `ultralight_v3` - **Production-Ready** UltraLight v3 (Clean Architecture)
- `ultralight_v2` - Legacy UltraLight v2 (LovyanGFX)
- `ultralight` - Legacy UltraLight v1 (TFT_eSPI)

### Configuration (UltraLight v3)

Alle Settings in `src/ultralight_v3/core/config.h`:

```cpp
// Display
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// BLE Scanner
#define BLE_SCAN_INTERVAL 100
#define BLE_SCAN_WINDOW   99
#define BLE_RSSI_THRESHOLD -100
#define BLE_UUID_PREFIX "c3:00:"

// Lap Detection
#define DEFAULT_LAP_RSSI_NEAR -65
#define DEFAULT_LAP_RSSI_FAR  -80
```

### BLE Backend Selection

In `config.h` BLE Backend wählen:

```cpp
// Uncomment ONE of these (or none for NimBLE - default):
// #define BLE_USE_ESP32       // ESP32 BLE Arduino
// #define BLE_USE_ESP_IDF     // ESP-IDF Native
// Default: NimBLE-Arduino (best performance)
```

## Testing

### Unit Tests (lokal)

```bash
pio test
```

### Hardware Tests

1. Flash Firmware
2. Serial Monitor öffnen
3. Debug-Output prüfen
4. Hardware-Funktionen testen

## Troubleshooting

**Upload schlägt fehl:**
- USB-Kabel prüfen
- Richtigen Port wählen: `pio device list`
- Boot-Button beim Upload halten (ESP32)

**LoRa funktioniert nicht:**
- Frequenz prüfen (868MHz EU / 915MHz US)
- Antenne korrekt angeschlossen?
- SPI Pins prüfen

**BLE Scanner findet keine Beacons:**
- Beacons eingeschaltet & Batterie?
- Reichweite <10m?
- UUID korrekt konfiguriert?

## Libraries

### UltraLight v3 Dependencies:
- **LovyanGFX** (^1.2.7) - Display Treiber
- **LVGL** (^8.4.0) - UI Framework
- **NimBLE-Arduino** (^1.4.0) - BLE Scanning (Standard)
- **ArduinoJson** (^7.0.0) - JSON Parsing
- **SdFat** (^2.2.2) - SD Card mit besserer Performance
- **SPI, SD** - Arduino Standard Libraries

### Legacy Dependencies:
- **TFT_eSPI** - Display (v1)
- **RadioHead** - LoRa (FullBlown)
- **FastLED** - WS2812 LED (FullBlown)
- **PubSubClient** - MQTT (FullBlown)

## Architecture (UltraLight v3)

### Clean Architecture Layers:

1. **Core** - System Management
   - SystemManager: Watchdog, Initialization
   - TaskManager: Task Scheduling
   - MemoryManager: Heap Monitoring

2. **Hardware** - Abstraction Layer
   - Display: LovyanGFX Wrapper
   - Touch: XPT2046 Handler
   - SD Card: File Operations
   - BLE: Multiple Backend Support

3. **Services** - Business Logic
   - BeaconService: BLE Scanning & Filtering
   - LapCounterService: Race Logic
   - PersistenceService: NVS Storage
   - DataLoggerService: CSV Logging

4. **UI** - User Interface
   - LVGL8 Screens
   - Navigation System
   - Widgets & Themes

### Service Communication:

```
UI (Screens) → Services → Hardware
     ↓            ↓          ↓
  Navigation   Logic      I/O
```



