# ESP32-2432S028 Display Versionen

## Varianten-Übersicht

### ESP32-2432S028R v1 (Original - Micro-USB, ILI9341)
- **Display:** ILI9341 SPI
- **Touch:** XPT2046 (Resistiv)
- **Touch Pins:** CS=33, via SPI
- **Touch Library:** TFT_eSPI built-in
- **Erkennung:** Mit Fingernagel drücken, fester Druck nötig
- **Problem:** U4 IC Design-Fehler (externe Flash)

### ESP32-2432S028R v2 (Micro-USB, ILI9341, korrigiert)
- **Display:** ILI9341 SPI
- **Touch:** XPT2046 (Resistiv)
- **Touch Pins:** CS=33, via SPI
- **Touch Library:** TFT_eSPI built-in
- **Änderungen:** U4 IC entfernt, **External Pin Assign changed!**
- **WICHTIG:** Pin-Belegungen für externe Anschlüsse geändert!

### ESP32-2432S028R v3 (USB-C, ST7789)
- **Display:** ST7789 SPI (MODE 3)
- **Touch:** XPT2046 (Resistiv) oder GT911/FT6236 (Kapazitiv)
- **Touch Pins:** 
  - Resistiv: CS=33, via SPI
  - Kapazitiv: SDA=33, SCL=32, INT=21, RST=25
- **Touch Library:** TFT_eSPI (Resistiv) oder GT911/FT6336U (Kapazitiv)
- **Besonderheit:** Einige Versionen haben zwei USB-Ports (Type-C + Micro-USB)

## Wie erkenne ich meine Version?

1. **USB Port:** 
   - Micro-USB → R (Original)
   - USB-C → Rv2 oder Rv3

2. **Touch-Verhalten:**
   - Fester Druck nötig → Resistiv (R)
   - Leichte Berührung → Kapazitiv (Rv2/Rv3)

3. **Chip-Markierung:**
   - Auf Rückseite neben Touch-Connector nachsehen
   - GT911 → Rv2
   - FT6236/FT6336U → Rv3

## Touch Pin Mapping

### Resistiv (R - XPT2046)
```
TOUCH_CS   = 33
TOUCH_IRQ  = 36 (optional)
Via SPI (shared with display)
```

### Kapazitiv (Rv2/Rv3 - GT911/FT6236)
```
TOUCH_SDA  = 33
TOUCH_SCL  = 32
TOUCH_INT  = 21
TOUCH_RST  = 25
Via I2C (separate bus)
```

## Display Specs

### v1/v2 (ILI9341)
- **Display:** ILI9341 SPI, 2.8" 240x320
- **TFT Pins:** SCK=14, RS/DC=2, CS=15, SDI/MOSI=13, SDO/MISO=12, BL=21

### v3 (ST7789)
- **Display:** ST7789 IPS SPI (MODE 3), 2.8" 320x240
- **TFT Pins:** SCK=14, RS/DC=2, CS=15, SDI/MOSI=13, SDO/MISO=12, BL=21

## SD Card Pins (alle Versionen)
- **SD Card:** CS=5, MOSI=23, MISO=19, SCLK=18 (VSPI standard pins)
- **⚠️ WICHTIG:** In früheren Dokumentationen waren MOSI und MISO vertauscht!
- **Referenz:** [macsbug Blog](https://macsbug.wordpress.com/2022/08/17/esp32-2432s028/)
- **RGB LED:** Red=4, Green=16, Blue=17

## Wichtige Pin-Belegungen

### SPI-Bus Übersicht:
- **TFT**: Verwendet Custom SPI Pins (13, 14, 12) - shared mit Touch
- **SD Card**: Verwendet VSPI Standard Pins (23, 19, 18) - separater Bus
- **Touch**: Shared mit TFT oder separate Pins je nach Variante

### Pin-Konflikte:
⚠️ **SPI-Konflikt zwischen Touch und SD:**
- TFT und SD verwenden unterschiedliche SPI-Busse (kein Hardware-Konflikt)
- **ABER:** ESP32 hat nur EIN globales SPI-Objekt
- Touch ruft `SPI.begin()` mit Touch-Pins (32, 39, 25) auf
- SD benötigt SPI mit SD-Pins (23, 19, 18)
- Wenn Touch zuerst initialisiert wird, funktioniert SD nicht
- Wenn SD zuerst initialisiert wird, funktioniert Touch nicht
- **Status:** Konflikt noch nicht gelöst, Test-Firmware zeigt dass SD Hardware funktioniert

## Revision-Unterschiede (nach macsbug Blog)

### Bekannte Revisionen:
1. **v1 (2022.08.17)**: ILI9341, U4 Flash IC Design-Fehler
2. **v2 (2022.09.22)**: ILI9341, U4 IC entfernt, Pin-Belegungen geändert
3. **v3 (2024.01.20)**: ST7789, zwei USB-Ports möglich
4. **v4-v7**: Weitere Iterationen

### Wichtige Hinweise:
- **Pin-Änderungen:** v2 hatte "External Pin Assign change" - externe Pins können anders sein
- **Dokumentationsfehler:** Ursprüngliche Dokumentation hatte MOSI/MISO vertauscht (rev 6 korrigiert)
- **Referenz:** Siehe [macsbug Blog](https://macsbug.wordpress.com/2022/08/17/esp32-2432s028/) für detaillierte Informationen

