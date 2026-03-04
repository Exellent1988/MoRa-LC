# Wokwi Simulation – MoRa-LC UltraLight v3

Simulation des ESP32-2432S028 (Cheap Yellow Display) mit ST7789, XPT2046 Touch und SD-Karte.

## Wichtig: Dateiposition

Die Wokwi-Extension sucht `diagram.json` und `wokwi.toml` **nur im Workspace-Root**.  
Diese Dateien liegen daher im **Projekt-Root** (MoRa-LC/), nicht hier. Dieser Ordner dient der Dokumentation.

## Ablauf

1. **Projekt-Root öffnen** (MoRa-LC/, nicht wokwi/)
2. **Firmware bauen:** `cd firmware && pio run -e ultralight_v3`
3. **Simulation starten:** `F1` → `Wokwi: Start Simulator`

## Hinweis

`wokwi-st7789` und `wokwi-xpt2046` sind möglicherweise keine Standard-Parts. Bei Fehlern siehe [Wokwi Custom Chips](https://docs.wokwi.com/chips-api/getting-started).
