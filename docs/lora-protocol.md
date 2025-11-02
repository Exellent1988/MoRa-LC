# MoRa-LC LoRa Kommunikations-Protokoll

**Version:** 1.0  
**Letzte Änderung:** 2025-11-01

## Übersicht

Das MoRa-LC LoRa-Protokoll definiert die Kommunikation zwischen:
- **Beacons** (auf Mofas) → **Gateways**
- **Zentrale/Gateways** → **Displays**
- **Zentrale** → **Beacons** (Commands)

## LoRa-Parameter

| Parameter | Wert | Bemerkung |
|-----------|------|-----------|
| Frequenz (EU) | 868 MHz | ISM-Band Europa |
| Frequenz (US) | 915 MHz | ISM-Band USA |
| Bandwidth | 125 kHz | Standard |
| Spreading Factor | 7-9 | SF7 = schnell, SF9 = Reichweite |
| Coding Rate | 4/5 | Standard |
| TX Power | 17 dBm | Maximal |
| Sync Word | 0x12 | Private Network |
| Preamble | 8 Symbole | Standard |

**Reichweite:**
- SF7: ~500m (Urban), ~2km (Open Field)
- SF9: ~1km (Urban), ~5km (Open Field)

**Airtime pro Nachricht:**
- SF7, 28 bytes: ~40ms
- SF9, 28 bytes: ~150ms

## Nachrichtentypen

### 0x01: Telemetry (Beacon → Gateway)

**Zweck:** Regelmäßige Übertragung von Beacon-Status

**Frequenz:** Alle 1-2 Sekunden

**Größe:** 28 bytes

```c
struct BeaconTelemetryMsg {
    uint8_t messageType;          // 0x01
    uint8_t beaconId;             // 1-99
    uint32_t timestamp;           // millis()
    uint8_t batteryPercent;       // 0-100%
    int8_t rssiGateway1;          // dBm (-128 bis 127)
    int8_t rssiGateway2;
    int8_t rssiGateway3;
    int8_t rssiGateway4;
    int16_t accelX;               // mg (Milli-G)
    int16_t accelY;
    int16_t accelZ;
    uint8_t lastCheckpointId;     // 0 = none
    uint32_t checkpointTimestamp;
    uint8_t checksum;
};
```

**Beispiel:**
```
01 05 00 00 12 34 5A B8 BC C0 C4 00 10 00 20 03 00 00 34 56 8F
│  │  │──────────┘ │  │  │  │  │  │  │  │  │  │  │  │──────────┘ │
│  │  Timestamp    │  │  │  │  │  │  │  │  │  │  │  Checkpoint  Checksum
│  │               │  R  R  R  R  Accel X Y  Z  │  Timestamp
│  Beacon ID       Battery  S  S  S  S           Last CP ID
Type               %         S  S  S  S
                             I  I  I  I
```

### 0x02: Checkpoint (Beacon → Gateway)

**Zweck:** Meldung bei Checkpoint-Durchfahrt

**Frequenz:** Event-basiert

**Größe:** 16 bytes

```c
struct BeaconCheckpointMsg {
    uint8_t messageType;          // 0x02
    uint8_t beaconId;             // 1-99
    uint32_t timestamp;           // millis()
    uint8_t checkpointId;         // 1-255
    char checkpointUUID[6];       // Erste 6 hex chars
    int8_t rssi;                  // dBm
    uint8_t checksum;
};
```

**Checkpoint UUID Encoding:**
- Volle UUID: `AABBCCDD-1234-5678-90AB-CDEF12345678`
- Gesendet: `AABBCC` (erste 6 chars)
- Backend rekonstruiert volle UUID aus Mapping

### 0x03: Crash Alert (Beacon → Gateway)

**Zweck:** Sturz-Erkennung Alarm

**Frequenz:** Event-basiert (IMU >2.5G)

**Größe:** 12 bytes

```c
struct BeaconCrashMsg {
    uint8_t messageType;          // 0x03
    uint8_t beaconId;             // 1-99
    uint32_t timestamp;           // millis()
    int16_t impactAccelX;         // mg
    int16_t impactAccelY;
    int16_t impactAccelZ;
    uint8_t checksum;
};
```

**Trigger-Bedingungen:**
- Gesamtbeschleunigung > 2500 mg (2.5G)
- ODER: Plötzliche Orientierungsänderung >90°

### 0x10: Command (Gateway/Central → Beacon)

**Zweck:** Fernsteuerung von Beacons

**Größe:** 8 bytes

```c
struct BeaconCommandMsg {
    uint8_t messageType;          // 0x10
    uint8_t targetBeaconId;       // 0 = broadcast
    uint8_t command;              // Command ID
    uint32_t param;               // Parameter
    uint8_t checksum;
};
```

**Commands:**

| Command | ID | Parameter | Beschreibung |
|---------|----|-----------|--------------| 
| PIEZO_BEEP | 0x01 | Duration (ms) | Piezo für X ms aktivieren |
| RESET | 0x02 | - | Beacon neu starten |
| SLEEP | 0x03 | Duration (ms) | Deep-Sleep für X ms |
| CONFIG_INTERVAL | 0x10 | Interval (ms) | Telemetrie-Intervall ändern |
| CONFIG_LORA_SF | 0x11 | SF (7-12) | Spreading Factor ändern |

**Beispiel - Piezo 500ms:**
```
10 05 01 F4 01 00 00 XX
│  │  │  │───────────┘ │
│  │  │  Parameter     Checksum
│  │  Command (PIEZO_BEEP)
│  Target Beacon ID (5)
Type (COMMAND)
```

### 0x20: Display Command (Central → Display)

**Zweck:** LED-Display Steuerung

**Größe:** 12 bytes

```c
struct DisplayCommandMsg {
    uint8_t messageType;          // 0x20
    uint8_t displayId;            // 1-99
    uint8_t mode;                 // Display mode
    uint16_t value;               // Mode-specific
    uint32_t color;               // RGB (optional)
    uint8_t checksum;
};
```

**Modi:**

| Mode | ID | Value | Color | Beschreibung |
|------|----|----|-------|--------------|
| OFF | 0x00 | - | - | Display aus |
| COUNTDOWN | 0x01 | Minutes | - | Restzeit anzeigen |
| TRAFFIC | 0x02 | 0=R, 1=Y, 2=G | RGB | Ampel-Farbe |
| WARNING | 0x03 | Blink rate (Hz) | RGB | Warnung blinken |
| TEXT | 0x04 | Text ID | - | Text-Scroll |

**Beispiel - Ampel Grün:**
```
20 01 02 02 00 00 FF 00 00 XX
│  │  │  │  │  │──────────┘ │
│  │  │  │  │  Color (Green) Checksum
│  │  │  │  Value (GREEN=2)
│  │  │  Mode (TRAFFIC)
│  │  Display ID (1)
│  Type (DISPLAY_CMD)
```

### 0xFF: Acknowledgment

**Zweck:** Bestätigung kritischer Nachrichten

**Größe:** 4 bytes

```c
struct AckMsg {
    uint8_t messageType;          // 0xFF
    uint8_t ackForType;           // Original message type
    uint8_t senderId;             // Who sends ACK
    uint8_t checksum;
};
```

**Verwendung:**
- Optional bei kritischen Messages (Crash, Commands)
- Timeout: 500ms
- Retry: Max 3x

## Collision Avoidance

**Problem:** Multiple Beacons senden gleichzeitig → Kollision

**Lösung:** Slotted ALOHA mit Random Offset

```
Beacon 1: [TX]----1s----[TX]----1s----[TX]
Beacon 2: --[TX]----1s----[TX]----1s----[TX]
Beacon 3: ----[TX]----1s----[TX]----1s----[TX]

Offset: Random 0-200ms pro Beacon
```

**Erfolgsrate:**
- 10 Beacons @ 1Hz: ~90%
- 20 Beacons @ 1Hz: ~75%
- 10 Beacons @ 2s: ~98%

## Checksum

**Algorithmus:** Simple XOR Checksum

```c
uint8_t checksum = 0;
for (size_t i = 0; i < length - 1; i++) {
    checksum ^= data[i];
}
```

**Vorteil:** Schnell, stromsparend  
**Nachteil:** Erkennt keine 2-Bit-Fehler  
**Alternative:** CRC8 (später)

## Timing-Diagramm

```
Beacon                Gateway               Backend
  │                     │                     │
  ├──[Telemetry]───────>│                     │
  │                     ├──[MQTT Publish]────>│
  │                     │                     │
  ├──[Checkpoint]──────>│                     │
  │                     ├──[MQTT Publish]────>│
  │                     │                     ├──[Process Lap]
  │                     │                     │
  │                     │<──[MQTT Subscribe]──┤
  │<──[Command: Piezo]──┤                     │
  │                     │                     │
  ├──[ACK]─────────────>│                     │
```

## Error Handling

**Checksum Error:**
- Nachricht verwerfen
- Log Error
- Keine Retry (nächste Telemetrie kommt in 1s)

**Timeout (Command/ACK):**
- Retry nach 500ms
- Max 3 Retries
- Dann: Error Log, weiter

**Unknown Message Type:**
- Log Warning
- Verwerfen
- Keine Aktion

## Bandwidth-Kalkulation

**Ein Beacon:**
- Telemetry: 28 bytes @ 1Hz = 28 B/s
- Checkpoints: 16 bytes @ ~0.05Hz (alle 20s) = 0.8 B/s
- **Total: ~29 B/s**

**10 Beacons:**
- Total: ~290 B/s
- Mit LoRa SF7: ~2500 B/s möglich
- **Auslastung: ~12%** ✅

**20 Beacons:**
- Total: ~580 B/s
- **Auslastung: ~23%** ✅

## Power Consumption

**LoRa TX (ESP32 + RFM95):**
- Idle: ~10mA
- TX (28 bytes, SF7): ~120mA for 40ms = 1.3mAh
- Deep-Sleep: ~0.05mA

**Mit 1Hz Telemetry:**
- TX: 1.3mAh * 3600/hour = ~5mAh/h
- Idle + BLE: ~50mA = 50mAh/h
- **Total: ~55mAh/h**

**1500mAh Akku:** ~27 Stunden  
**Mit 2s Interval:** ~35 Stunden  
**Mit Deep-Sleep:** >50 Stunden

## Best Practices

### Beacons
1. Random TX Offset implementieren (0-200ms)
2. Telemetry-Intervall anpassbar (1-5s)
3. Checksum immer prüfen
4. Bei schwacher Batterie (<20%) Intervall erhöhen

### Gateways
5. Alle Messages loggen (Debug)
6. Duplicate Detection (gleiche Msg von mehreren GWs)
7. RSSI für jedes Beacon tracken
8. NTP für Zeitstempel-Sync

### Backend
9. Messages validieren (Checksum, Plausibilität)
10. Alte Daten verwerfen (>5s)
11. Beacon-Timeouts erkennen (>10s keine Msg)
12. Leaderboard nur auf Checkpoint-Msgs basieren

## Testing

**Unit-Tests:**
```cpp
// Test Checksum
BeaconTelemetryMsg msg = LoRaProtocol::createTelemetryMsg(1);
assert(LoRaProtocol::verifyChecksum((uint8_t*)&msg, sizeof(msg)));

// Test Parsing
uint8_t data[28] = {...};
BeaconTelemetryMsg parsed;
assert(LoRaProtocol::parseMessage(data, 28, &parsed));
assert(parsed.beaconId == 1);
```

**Integration-Test:**
1. Beacon sendet Telemetry
2. Gateway empfängt & forwarded via MQTT
3. Backend verarbeitet & updated DB
4. Frontend zeigt Live-Update

## Zukünftige Erweiterungen

**V2.0:**
- CRC16 statt XOR Checksum
- Message Sequence Numbers (Duplicate Detection)
- Encryption (AES128)
- Compressed Telemetry (reduziere auf 16 bytes)

**V3.0:**
- Bidirektionale Communication (ACKs für alles)
- Multi-Hop (Beacons als Repeater)
- Mesh-Network Support



