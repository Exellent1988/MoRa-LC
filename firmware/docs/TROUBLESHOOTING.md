# PlatformIO Troubleshooting

## Problem: "Resolving dependencies..." hängt

### Lösung 1: Cache cleanup
```bash
cd firmware
rm -rf .pio
# Dann in PlatformIO: Build nochmal versuchen
```

### Lösung 2: Manueller Library Download

Falls der automatische Download nicht funktioniert:

```bash
cd firmware

# TFT_eSPI manuell installieren
~/.platformio/penv/bin/pio pkg install --library "bodmer/TFT_eSPI" -e ultralight

# NimBLE manuell installieren
~/.platformio/penv/bin/pio pkg install --library "h2zero/NimBLE-Arduino" -e ultralight

# ArduinoJson manuell installieren
~/.platformio/penv/bin/pio pkg install --library "bblanchon/ArduinoJson" -e ultralight
```

### Lösung 3: Proxy/Firewall Check

Prüfe ob Firewall/Antivirus PlatformIO blockiert:
- Temporär Firewall ausschalten
- PlatformIO zu Firewall Whitelist hinzufügen
- Proxy Settings prüfen (falls im Firmennetzwerk)

### Lösung 4: Verbose Output

Sieh genau wo es hängt:
```bash
cd firmware
~/.platformio/penv/bin/pio run -e ultralight -v
```

### Lösung 5: Alternative Registry

Falls GitHub Registry down ist, alternative nutzen:
```bash
~/.platformio/penv/bin/pio settings set check_libraries_interval 0
~/.platformio/penv/bin/pio settings set check_platformio_interval 0
```

## Problem: "Unknown board 'esp32dev'"

### Lösung: ESP32 Platform installieren
```bash
~/.platformio/penv/bin/pio pkg install --platform "espressif32"
```

## Problem: Compile Errors

### Häufige Fehler:

**1. "NimBLEDevice.h: No such file"**
→ NimBLE nicht installiert, siehe Lösung 2 oben

**2. "TFT_eSPI.h: No such file"**
→ TFT_eSPI nicht installiert, siehe Lösung 2 oben

**3. "undefined reference to..."**
→ Library nicht richtig gelinkt, Cache cleanen und neu builden

## SD-Karte wird nicht erkannt (ESP32-2432S028)

### Status (Stand: Aktuell)
- ✅ **SD-Karte funktioniert** in isolierter Test-Firmware (`test_sd/`)
- ❌ **SD-Karte funktioniert NICHT** in Haupt-Firmware mit Touch
- ✅ **Hardware ist OK**: Pins CS=5, MOSI=23, MISO=19, SCLK=18 sind korrekt
- ⚠️ **Problem**: SPI-Konflikt zwischen Touch und SD

### Problem: SPI-Konflikt zwischen Touch und SD

**Root Cause:**
ESP32 hat nur EIN globales SPI-Objekt. Die Initialisierungsreihenfolge ist kritisch:
1. Touch ruft `SPI.begin(25, 39, 32, 33)` auf (Touch-Pins)
2. SD ruft `SD.begin(5)` auf, benötigt aber SPI mit SD-Pins (23, 19, 18)
3. Da SPI bereits mit Touch-Pins initialisiert ist, funktioniert SD nicht

**Test-Firmware beweist:**
- SD funktioniert perfekt wenn Touch NICHT initialisiert wird
- Card Type: 2 (SDHC), Size: 120 MB - alles funktioniert!

**Aktueller Stand:**
- Initialisierungsreihenfolge geändert: SD vor Touch
- SD wird initialisiert, aber Touch funktioniert dann nicht mehr
- Problem: Beide können nicht gleichzeitig funktionieren mit derzeitiger Implementierung

### Lösung (in Arbeit)

**Option 1: Separate SPI-Instanzen**
- ESP32 hat VSPI und HSPI - möglicherweise können beide verwendet werden
- Touch auf einem Bus, SD auf dem anderen

**Option 2: SPI-Management**
- SD initialisieren und dann SPI nicht neu initialisieren für Touch
- Touch-Library anpassen, um SD-SPI nicht zu stören

**Option 3: Lazy Initialization**
- SD nur initialisieren wenn tatsächlich benötigt
- Touch für UI immer verfügbar

### Software bereits getestet
- ✅ CS-Pin Software-Test (Pin funktioniert)
- ✅ MISO-Pin 12 (Standard für ESP32)
- ✅ MISO-Pin 19 (Alternative)
- ✅ SPI-Frequenz 4 MHz
- ✅ SPI-Frequenz 1 MHz (sehr langsam)
- ✅ Alle Pin-Kombinationen
- ✅ Multiple Initialisierungsversuche
- ✅ Isolierte Test-Firmware (funktioniert!)

### Test-Firmware
Eine minimale Test-Firmware ist verfügbar unter `test_sd/`:
```bash
cd test_sd
platformio run --target upload --target monitor
```
Diese Firmware testet nur SD-Karte ohne Touch und beweist, dass Hardware funktioniert.

### Workaround
Das System funktioniert ohne SD-Karte. Daten können in NVS (EEPROM) gespeichert werden.
Die Formatierungsfunktion ist in den Einstellungen verfügbar, falls SPI-Konflikt gelöst wird.

## Hilfe

Wenn nichts funktioniert, öffne ein Issue mit:
- PlatformIO Version: `pio --version`
- Verbose Build Output
- OS Version
- Firewall/Antivirus Software

