# ESP32-2432S028 Display Versionen

## Varianten-Übersicht

### ESP32-2432S028R (Original - Micro-USB)
- **Touch:** XPT2046 (Resistiv)
- **Touch Pins:** CS=33, via SPI
- **Touch Library:** TFT_eSPI built-in
- **Erkennung:** Mit Fingernagel drücken, fester Druck nötig

### ESP32-2432S028Rv2 (USB-C, GT911)
- **Touch:** GT911 (Kapazitiv)
- **Touch Pins:** SDA=33, SCL=32, INT=21, RST=25
- **Touch Library:** GT911 Arduino Library
- **Erkennung:** Leichte Berührung reicht

### ESP32-2432S028Rv3 (USB-C, FT6236)
- **Touch:** FT6236 / FT6336U (Kapazitiv)
- **Touch Pins:** SDA=33, SCL=32, INT=21, RST=25
- **Touch Library:** FT6336U Arduino Library
- **Erkennung:** Leichte Berührung reicht

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

## Display Specs (alle Versionen gleich)
- **Display:** ST7789 IPS 2.8" 320x240
- **TFT Pins:** MOSI=13, SCLK=14, CS=15, DC=2, BL=21
- **SD Card:** CS=5
- **RGB LED:** Red=4, Green=16, Blue=17

