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

## Hilfe

Wenn nichts funktioniert, öffne ein Issue mit:
- PlatformIO Version: `pio --version`
- Verbose Build Output
- OS Version
- Firewall/Antivirus Software

