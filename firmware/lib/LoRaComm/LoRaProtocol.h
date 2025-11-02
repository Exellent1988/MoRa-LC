#ifndef LORA_PROTOCOL_H
#define LORA_PROTOCOL_H

#include <Arduino.h>

/**
 * MoRa-LC LoRa Communication Protocol
 * 
 * Definiert Nachrichtenformate für LoRa-Kommunikation
 * zwischen Beacons, Gateways und Zentrale
 */

// ============================================================
// Protocol Constants
// ============================================================

#define LORA_FREQUENCY_EU 868E6      // 868 MHz (Europa)
#define LORA_FREQUENCY_US 915E6      // 915 MHz (USA)
#define LORA_BANDWIDTH 125E3         // 125 kHz
#define LORA_SPREADING_FACTOR 7      // SF7 (schnell, kurze Reichweite)
#define LORA_CODING_RATE 5           // 4/5
#define LORA_TX_POWER 17             // dBm
#define LORA_PREAMBLE_LENGTH 8

#define LORA_MAX_PAYLOAD 255         // bytes
#define LORA_SYNC_WORD 0x12          // Private network

// Message Types
#define MSG_TYPE_TELEMETRY      0x01  // Beacon → Gateway: Regular telemetry
#define MSG_TYPE_CHECKPOINT     0x02  // Beacon → Gateway: Checkpoint event
#define MSG_TYPE_CRASH          0x03  // Beacon → Gateway: Crash detected
#define MSG_TYPE_COMMAND        0x10  // Gateway → Beacon: Command
#define MSG_TYPE_DISPLAY_CMD    0x20  // Central → Display: Display command
#define MSG_TYPE_ACK            0xFF  // Acknowledgment

// ============================================================
// Message Structures
// ============================================================

/**
 * Beacon → Gateway: Telemetry Data
 * Sendet: Alle 1-2 Sekunden
 * Größe: ~28 bytes
 */
struct __attribute__((packed)) BeaconTelemetryMsg {
    uint8_t messageType;          // MSG_TYPE_TELEMETRY
    uint8_t beaconId;             // 1-99
    uint32_t timestamp;           // millis() auf Beacon
    
    // Power
    uint8_t batteryPercent;       // 0-100%
    
    // BLE RSSI zu Gateways (für Triangulation)
    int8_t rssiGateway1;          // dBm
    int8_t rssiGateway2;
    int8_t rssiGateway3;
    int8_t rssiGateway4;
    
    // IMU (optional)
    int16_t accelX;               // mg (milli-g)
    int16_t accelY;
    int16_t accelZ;
    
    // Checkpoint
    uint8_t lastCheckpointId;     // 0 = none
    uint32_t checkpointTimestamp; // millis()
    
    // Checksum
    uint8_t checksum;             // Simple XOR checksum
};

/**
 * Beacon → Gateway: Checkpoint Event
 * Sendet: Bei Checkpoint-Durchfahrt
 * Größe: ~16 bytes
 */
struct __attribute__((packed)) BeaconCheckpointMsg {
    uint8_t messageType;          // MSG_TYPE_CHECKPOINT
    uint8_t beaconId;             // 1-99
    uint32_t timestamp;           // millis()
    
    uint8_t checkpointId;         // 1-255
    char checkpointUUID[6];       // Gekürzte UUID (erste 6 hex chars)
    int8_t rssi;                  // dBm zum Checkpoint
    
    uint8_t checksum;
};

/**
 * Beacon → Gateway: Crash Alert
 * Sendet: Bei Sturz-Erkennung (IMU >2.5G)
 * Größe: ~12 bytes
 */
struct __attribute__((packed)) BeaconCrashMsg {
    uint8_t messageType;          // MSG_TYPE_CRASH
    uint8_t beaconId;             // 1-99
    uint32_t timestamp;           // millis()
    
    int16_t impactAccelX;         // mg
    int16_t impactAccelY;
    int16_t impactAccelZ;
    
    uint8_t checksum;
};

/**
 * Gateway/Central → Beacon: Command
 * Sendet: Bei Bedarf (z.B. Piezo aktivieren)
 * Größe: ~8 bytes
 */
struct __attribute__((packed)) BeaconCommandMsg {
    uint8_t messageType;          // MSG_TYPE_COMMAND
    uint8_t targetBeaconId;       // 0 = broadcast, 1-99 = specific
    uint8_t command;              // Command ID
    uint32_t param;               // Command parameter
    uint8_t checksum;
};

// Beacon Commands
#define CMD_PIEZO_BEEP       0x01  // param = duration ms
#define CMD_RESET            0x02  // param = unused
#define CMD_SLEEP            0x03  // param = duration ms
#define CMD_CONFIG_INTERVAL  0x10  // param = interval ms
#define CMD_CONFIG_LORA_SF   0x11  // param = SF (7-12)

/**
 * Central → Display: Display Command
 * Sendet: Bei Display-Updates
 * Größe: ~12 bytes
 */
struct __attribute__((packed)) DisplayCommandMsg {
    uint8_t messageType;          // MSG_TYPE_DISPLAY_CMD
    uint8_t displayId;            // 1-99
    uint8_t mode;                 // Display mode
    uint16_t value;               // Mode-specific value
    uint32_t color;               // RGB color (optional)
    uint8_t checksum;
};

// Display Modes
#define DISPLAY_MODE_OFF        0x00
#define DISPLAY_MODE_COUNTDOWN  0x01  // value = minutes remaining
#define DISPLAY_MODE_TRAFFIC    0x02  // value = color (red/yellow/green)
#define DISPLAY_MODE_WARNING    0x03  // value = blink rate
#define DISPLAY_MODE_TEXT       0x04  // value = text ID

// Traffic Light Colors
#define TRAFFIC_RED     0
#define TRAFFIC_YELLOW  1
#define TRAFFIC_GREEN   2

/**
 * Any → Any: Acknowledgment
 * Sendet: Optional bei kritischen Messages
 * Größe: ~4 bytes
 */
struct __attribute__((packed)) AckMsg {
    uint8_t messageType;          // MSG_TYPE_ACK
    uint8_t ackForType;           // Original message type
    uint8_t senderId;             // Who sends ACK
    uint8_t checksum;
};

// ============================================================
// Helper Functions
// ============================================================

class LoRaProtocol {
public:
    // Checksum
    static uint8_t calculateChecksum(const uint8_t* data, size_t length);
    static bool verifyChecksum(const uint8_t* data, size_t length);
    
    // Message Creation
    static BeaconTelemetryMsg createTelemetryMsg(uint8_t beaconId);
    static BeaconCheckpointMsg createCheckpointMsg(uint8_t beaconId, uint8_t checkpointId, 
                                                    const char* uuid, int8_t rssi);
    static BeaconCrashMsg createCrashMsg(uint8_t beaconId, int16_t x, int16_t y, int16_t z);
    static BeaconCommandMsg createCommandMsg(uint8_t targetId, uint8_t cmd, uint32_t param);
    static DisplayCommandMsg createDisplayMsg(uint8_t displayId, uint8_t mode, uint16_t value);
    
    // Message Parsing
    static uint8_t getMessageType(const uint8_t* data, size_t length);
    static bool parseMessage(const uint8_t* data, size_t length, void* msgStruct);
    
    // Debug
    static void printMessage(const uint8_t* data, size_t length);
};

#endif // LORA_PROTOCOL_H



