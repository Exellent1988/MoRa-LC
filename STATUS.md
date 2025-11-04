# MoRa-LC - Implementierungs-Status

**Letztes Update:** 2025-11-04 — UltraLight v3 UI & Logging Refresh  
**Version:** 0.1.0 (Prototyp)

## UltraLight v3 (aktueller Stand ~80 %)

- ✅ Konsistentes UI-Theme (Fonts & Farben) auf allen LVGL-Screens
- ✅ SD-Logging via SdFat (Race Start/Stop, Lap-Events, Queue-Flushing)
- ✅ Settings Screen: BLE-Konfiguration (RSSI-Slider & Scan-Switch) + Team-Persistenz
- ✅ Race Flow: Stop → Results Navigation, Leaderboard mit Highlights
- ✅ Teams Screen: Lösch-Bestätigung über LVGL-Dialog
- 🚧 Icon-System & visuelle Feinjustierung
- 🚧 Farb-/Kontrasttests auf echter Hardware

## Nächste Schritte (UltraLight)

1. Hardware-Test der neuen UI- und Logging-Funktionen
2. Race-Results Layout weiter polieren (Icons, Zusammenfassung)
3. Fontgrößen & Kontraste auf dem Display verifizieren
4. Freien SD-Speicher über SdFat ermitteln und im UI anzeigen

## FullBlown System

- Firmware, Backend und Frontend bleiben vorerst offen (0 %).
- Fokus liegt weiterhin auf UltraLight v3, bis Field-Tests erfolgreich abgeschlossen sind.

## Build-Status

- ⚠️ PlatformIO CLI ist auf der aktuellen VM nicht installiert (`pio`/`platformio` nicht gefunden`).  
  Bitte Builds lokal oder in einer Umgebung mit installiertem PlatformIO per `pio run -e ultralight_v3` ausführen.

## Bekannte Einschränkungen

- Kein RTC – CSV-Logs nutzen `millis()` als Zeitbasis.
- `SDCard::getFreeSpace()` liefert derzeit `0` (SdFat-Volumenabfrage steht noch aus).
- Icon-Bitmaps weiterhin Platzhalter.
- Keine automatisierten Tests vorhanden.

## Dokumentation

- `README.md` (Root) und Firmware-Dokumente werden nach dem ersten Hardware-Test aktualisiert.
- Diese Statusseite bildet den Fortschritt der UltraLight v3 Firmware ab.
