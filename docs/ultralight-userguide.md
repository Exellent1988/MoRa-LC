# MoRa-LC UltraLight - Benutzeranleitung

**Version:** 0.1.0  
**Hardware:** CheapYellow Display (ESP32-2432S028)

## Übersicht

Das MoRa-LC UltraLight ist ein eigenständiges Lap-Counter System für Mofarennen. Es benötigt nur:
- 1x CheapYellow Display (~15€)
- Nx BLE Beacons (Xiaomi Mi Beacon, ~3€/Stück)
- 1x Powerbank

Kein Laptop, keine zusätzliche Hardware nötig!

## Hardware-Setup

### 1. CheapYellow Display vorbereiten

**Firmware flashen:**
```bash
cd firmware/ultralight
pio run -t upload
```

**SD-Karte einsetzen:**
- FAT32 formatiert
- Min. 1GB empfohlen
- Daten werden in `/races/` gespeichert

### 2. BLE Beacons vorbereiten

**Xiaomi Mi Beacons:**
1. Batterie (CR2032) einlegen
2. Aufkleber mit Nummer anbringen (#1, #2, #3...)
3. An Mofa befestigen (Magnethalterung oder Kabelbinder)
4. Einschalten → blinkt kurz

**Position:** Möglichst zentral am Mofa, nicht direkt am Metall.

### 3. Display aufstellen

- An Start/Ziel Position aufstellen
- Höhe: ~1-2m (optimale BLE-Reichweite)
- Powerbank anschließen
- Display einschalten

## Ersteinrichtung

### 1. Teams anlegen

```
Home → "Teams verwalten" → "+ Neues Team"
```

Für jedes Team:
1. Team-Name eingeben (z.B. "Blitz-Mofas")
2. "Beacon zuordnen" antippen
3. **Methode 1 (empfohlen):** 
   - Mofa mit Beacon direkt vor Display halten (<1m)
   - Warten bis "Nächster Beacon" angezeigt wird
   - "Diesen Beacon zuordnen" antippen
4. **Methode 2:**
   - Alle Beacons eingeschaltet lassen
   - Beacon aus Liste auswählen
5. "Speichern"

**Tipp:** Teams können auch während einem Rennen verwaltet werden!

### 2. Rennen einrichten

```
Home → "Neues Rennen"
```

1. **Name:** Rennen-Name eingeben (z.B. "Mofa-Cup Lauf 1")
2. **Dauer:** Minuten einstellen (Standard: 60 Min)
3. **Teams:** Automatisch alle angelegten Teams
4. **"Rennen starten"**

## Während des Rennens

### Display-Anzeige

```
┌──────────────────────────┐
│ Zeit: 42:15              │  ← Restzeit
├──────────────────────────┤
│ 1. Blitz-Mofas    12R    │  ← Rangliste
│    Letzte: 3:24.5        │     Live-Update
│                          │
│ 2. Speed Demons   12R    │
│    Letzte: 3:28.1        │
│                          │
│ [Pause]  [Stop]          │  ← Kontrolle
└──────────────────────────┘
```

### Rundenzählung

**Automatisch:**
- Mofa fährt am Display vorbei (< 3m)
- BLE Beacon wird erkannt
- Runde wird gezählt
- Zeit wird gemessen

**Erste "Runde":** Nur Startzeit, keine Runden-Zeit.

**Minimale Rundenzeit:** 10 Sekunden (Schutz vor Doppel-Erkennung)

### Buttons

**Pause:**
- Rennen anhalten (Zeit läuft nicht weiter)
- "Weitermachen" zum Fortsetzen
- "Rennen beenden" für finale Ergebnisse

**Stop:**
- Rennen sofort beenden
- Zeigt Ergebnisse
- Daten werden auf SD gespeichert

## Nach dem Rennen

### Ergebnisse anzeigen

```
Home → "Ergebnisse"
```

**Anzeige:**
- Finale Rangliste (nach Runden sortiert)
- Anzahl Runden pro Team
- Beste Rundenzeit
- Medaillen für Top 3 (🥇🥈🥉)

### Daten auf SD-Karte

**Automatisch gespeichert:**
```
/races/TIMESTAMP_RaceName.csv
```

**Format:**
```csv
Team ID,Team Name,Lap Number,Timestamp (ms),Duration (ms),Time of Day
1,Blitz-Mofas,1,65432,0,00:01:05
1,Blitz-Mofas,2,205678,140246,00:03:25
...
```

**Auswertung:**
- CSV in Excel/LibreOffice öffnen
- Oder auf PC kopieren für weitere Analyse

## Problemlösung

### "Kein Beacon gefunden"

**Ursachen:**
- Beacon ausgeschaltet / Batterie leer
- Zu weit entfernt (>3m)
- Beacon bereits anderem Team zugeordnet

**Lösung:**
- Beacon näher ans Display halten
- Batterie prüfen (CR2032)
- In "Teams verwalten" prüfen ob Beacon schon zugeordnet

### Runde wird nicht gezählt

**Ursachen:**
- Beacon nicht zugeordnet
- Zu schnell vorbeigefahren (< 10s)
- BLE-Scanner nicht aktiv

**Lösung:**
- Team-Zuordnung prüfen ("Teams verwalten")
- Langsamer vorbeifahren
- Display neu starten

### "SD: NO" im Status

**Ursachen:**
- Keine SD-Karte eingesetzt
- SD-Karte nicht formatiert (FAT32 nötig)
- SD-Karte defekt

**Lösung:**
- SD-Karte einsetzen
- FAT32 formatieren
- Andere SD-Karte testen

**Note:** Rennen funktionieren auch ohne SD, aber Daten gehen bei Neustart verloren!

### Display reagiert nicht

**Lösung:**
- Touch kalibrieren: Gerät neu starten
- Fest drücken (nicht nur antippen)
- Display aus Gehäuse nehmen (falls montiert)

### BLE findet Beacons nicht

**Lösung:**
- BLE-Scanner in "Einstellungen" aktivieren
- Beacon-Batterie prüfen
- Beacon näher halten (<5m)
- Gerät neu starten

## Tipps & Tricks

### Optimale Beacon-Platzierung

✅ **Gut:**
- Mittig am Mofa
- Unter Sitzbank
- An Kunststoff-Verkleidung

❌ **Schlecht:**
- Direkt am Motor (Vibration)
- An Metall-Rahmen (Dämpfung)
- Tief unten (schlechte Sichtlinie)

### Display-Position

- **Start/Ziel:** Display steht da wo Mofas vorbeifahren
- **Höhe:** 1-2m (optimale BLE-Reichweite)
- **Sichtlinie:** Möglichst freie Sicht zu Mofas
- **Entfernung:** Mofas fahren < 3m am Display vorbei

### Mehrere Rennen

- Teams bleiben gespeichert
- Für jedes Rennen: "Neues Rennen"
- Alte Ergebnisse unter "Ergebnisse"
- SD-Karte sammelt alle Rennen

### Batterie-Laufzeit

**Display (Powerbank):**
- 10000mAh Powerbank: ~8-10 Stunden
- Bei Pause: Display dimmen spart Strom

**Beacons (CR2032):**
- Xiaomi Mi Beacon: 6-12 Monate
- Warnung wenn Batterie < 20%

### Backup

**SD-Karte regelmäßig sichern:**
```bash
# Auf PC kopieren
cp /SD/races/*.csv ~/mora-backup/
```

### Multiple Teams gleichzeitig

- Max. 20 Teams konfigurierbar
- Max. 10 Mofas gleichzeitig auf Strecke (BLE-Limit)
- Bei mehr Teams: Staffel-Rennen machen

## Technische Spezifikationen

| Parameter | Wert |
|-----------|------|
| Display | 2.4" TFT 320x240 Touch |
| BLE Reichweite | 1-10m (optimal: <3m) |
| Rundenzählung | Automatisch, <1s Latenz |
| Minimale Rundenzeit | 10 Sekunden |
| Max. Teams | 20 |
| Max. gleichzeitig | 10 |
| SD-Karte | FAT32, min. 1GB |
| Stromversorgung | USB 5V, ~500mA |
| Laufzeit | 8-10h (10000mAh Powerbank) |
| Beacon-Typ | BLE iBeacon (Xiaomi kompatibel) |
| Beacon-Batterie | CR2032, 6-12 Monate |

## Weiterentwicklung

Geplante Features:
- On-Screen Tastatur (Text-Eingabe)
- Zahlen-Picker (Dauer-Eingabe)
- Beacon-Liste (Methode 2)
- WiFi Export (CSV Download)
- Zweites Display (Zuschauer-Anzeige)
- Strafen-System (Runden abziehen)

## Support

**Probleme melden:**
- GitHub Issues: [Link]
- E-Mail: [...]

**Dokumentation:**
- Haupt-README: `/README.md`
- Firmware-Details: `/firmware/README.md`
- Hardware-Info: `/hardware/`

---

**Version:** 0.1.0  
**Letzte Änderung:** 2025-11-01  
**Lizenz:** MIT



