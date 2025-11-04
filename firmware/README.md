# Firmware

ESP32 Firmware für alle Hardware-Komponenten des MoRa-LC Systems.

## Struktur

### Shared Libraries (`/lib/`)
Gemeinsame Module für beide Varianten:
- **BLEScanner** - BLE iBeacon Scanning und Erkennung
- **LapCounter** - Rundenzählung Algorithmus
- **DataLogger** - SD-Karte Logging (CSV)
- **LoRaComm** - LoRa Kommunikation (nur FullBlown)
- **IMUHandler** - IMU MPU6050 Integration (nur FullBlown)
- **PositionTracker** - Position Tracking (nur FullBlown)

### Firmware-Projekte

#### UltraLight v3 (`/ultralight_v3/`)
CheapYellow Display Firmware für standalone Betrieb.
**Clean Architecture Rewrite mit LVGL8**

**Hardware:** ESP32-2432S028 (CheapYellow)

**Features:**
- LVGL8 Touch Display UI (320x240)
- BLE Scanner für Beacon-Erkennung (NimBLE/ESP32/ESP-IDF)
- Team-Management UI mit Dialog-System
- Rundenzählung & Zeiten mit Best-Lap-Tracking
- SD-Karte Logging mit SdFat (CSV Export)
- Konsistentes Font- und Theme-System
- Vollständige Navigation zwischen Screens

**Build:**
```bash
cd firmware
pio run -e ultralight_v3 -t upload
```

#### UltraLight v2 (`/ultralight_v2/`)
Legacy Version (nicht mehr aktiv entwickelt).

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
# VS Code Extension installieren oder:
pip install platformio

# Build für spezifisches Environment:
cd beacon
pio run

# Upload:
pio run -t upload

# Serial Monitor:
pio device monitor
```

### Multi-Environment Config

Jedes Projekt hat eigene `platformio.ini` mit shared lib dependencies.

Beispiel:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    SPI
    Wire
    ../lib/BLEScanner
    ../lib/LapCounter
```

### Configuration

Beacon-ID und andere Settings in `src/config.h`:

```cpp
#define BEACON_ID 1        // Eindeutige ID (1-99)
#define LORA_FREQ 868E6    // 868MHz (EU)
#define BEACON_INTERVAL 1000  // ms
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

Verwendete externe Libraries:
- **RadioHead** - LoRa Kommunikation
- **FastLED** - WS2812 LED Steuerung
- **TFT_eSPI** - Display (UltraLight)
- **ArduinoJson** - JSON Parsing
- **PubSubClient** - MQTT Client
- **ESP32 BLE Arduino** - BLE Funktionen



