#include "LoRaProtocol.h"
#include <cstring>

// ============================================================
// Checksum Functions
// ============================================================

uint8_t LoRaProtocol::calculateChecksum(const uint8_t* data, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < length - 1; i++) {  // -1: exclude checksum byte itself
        checksum ^= data[i];
    }
    return checksum;
}

bool LoRaProtocol::verifyChecksum(const uint8_t* data, size_t length) {
    if (length < 2) return false;
    
    uint8_t calculated = calculateChecksum(data, length);
    uint8_t received = data[length - 1];
    
    return calculated == received;
}

// ============================================================
// Message Creation
// ============================================================

BeaconTelemetryMsg LoRaProtocol::createTelemetryMsg(uint8_t beaconId) {
    BeaconTelemetryMsg msg;
    memset(&msg, 0, sizeof(msg));
    
    msg.messageType = MSG_TYPE_TELEMETRY;
    msg.beaconId = beaconId;
    msg.timestamp = millis();
    
    // Checksum wird später hinzugefügt (wenn alle Daten gesetzt sind)
    
    return msg;
}

BeaconCheckpointMsg LoRaProtocol::createCheckpointMsg(uint8_t beaconId, uint8_t checkpointId,
                                                       const char* uuid, int8_t rssi) {
    BeaconCheckpointMsg msg;
    memset(&msg, 0, sizeof(msg));
    
    msg.messageType = MSG_TYPE_CHECKPOINT;
    msg.beaconId = beaconId;
    msg.timestamp = millis();
    msg.checkpointId = checkpointId;
    msg.rssi = rssi;
    
    // UUID auf erste 6 Zeichen kürzen
    if (uuid) {
        strncpy(msg.checkpointUUID, uuid, 6);
        msg.checkpointUUID[5] = '\0';
    }
    
    // Checksum
    msg.checksum = calculateChecksum((uint8_t*)&msg, sizeof(msg));
    
    return msg;
}

BeaconCrashMsg LoRaProtocol::createCrashMsg(uint8_t beaconId, int16_t x, int16_t y, int16_t z) {
    BeaconCrashMsg msg;
    memset(&msg, 0, sizeof(msg));
    
    msg.messageType = MSG_TYPE_CRASH;
    msg.beaconId = beaconId;
    msg.timestamp = millis();
    msg.impactAccelX = x;
    msg.impactAccelY = y;
    msg.impactAccelZ = z;
    
    // Checksum
    msg.checksum = calculateChecksum((uint8_t*)&msg, sizeof(msg));
    
    return msg;
}

BeaconCommandMsg LoRaProtocol::createCommandMsg(uint8_t targetId, uint8_t cmd, uint32_t param) {
    BeaconCommandMsg msg;
    memset(&msg, 0, sizeof(msg));
    
    msg.messageType = MSG_TYPE_COMMAND;
    msg.targetBeaconId = targetId;
    msg.command = cmd;
    msg.param = param;
    
    // Checksum
    msg.checksum = calculateChecksum((uint8_t*)&msg, sizeof(msg));
    
    return msg;
}

DisplayCommandMsg LoRaProtocol::createDisplayMsg(uint8_t displayId, uint8_t mode, uint16_t value) {
    DisplayCommandMsg msg;
    memset(&msg, 0, sizeof(msg));
    
    msg.messageType = MSG_TYPE_DISPLAY_CMD;
    msg.displayId = displayId;
    msg.mode = mode;
    msg.value = value;
    msg.color = 0x00FF00;  // Default: Green
    
    // Checksum
    msg.checksum = calculateChecksum((uint8_t*)&msg, sizeof(msg));
    
    return msg;
}

// ============================================================
// Message Parsing
// ============================================================

uint8_t LoRaProtocol::getMessageType(const uint8_t* data, size_t length) {
    if (length < 1) return 0;
    return data[0];
}

bool LoRaProtocol::parseMessage(const uint8_t* data, size_t length, void* msgStruct) {
    if (!data || !msgStruct || length < 2) {
        return false;
    }
    
    // Verify checksum
    if (!verifyChecksum(data, length)) {
        Serial.println("[LoRa] ERROR: Checksum failed");
        return false;
    }
    
    // Get message type
    uint8_t msgType = getMessageType(data, length);
    
    // Parse based on type
    switch (msgType) {
        case MSG_TYPE_TELEMETRY:
            if (length == sizeof(BeaconTelemetryMsg)) {
                memcpy(msgStruct, data, length);
                return true;
            }
            break;
            
        case MSG_TYPE_CHECKPOINT:
            if (length == sizeof(BeaconCheckpointMsg)) {
                memcpy(msgStruct, data, length);
                return true;
            }
            break;
            
        case MSG_TYPE_CRASH:
            if (length == sizeof(BeaconCrashMsg)) {
                memcpy(msgStruct, data, length);
                return true;
            }
            break;
            
        case MSG_TYPE_COMMAND:
            if (length == sizeof(BeaconCommandMsg)) {
                memcpy(msgStruct, data, length);
                return true;
            }
            break;
            
        case MSG_TYPE_DISPLAY_CMD:
            if (length == sizeof(DisplayCommandMsg)) {
                memcpy(msgStruct, data, length);
                return true;
            }
            break;
            
        case MSG_TYPE_ACK:
            if (length == sizeof(AckMsg)) {
                memcpy(msgStruct, data, length);
                return true;
            }
            break;
            
        default:
            Serial.printf("[LoRa] ERROR: Unknown message type: 0x%02X\n", msgType);
            return false;
    }
    
    Serial.printf("[LoRa] ERROR: Invalid message length for type 0x%02X (got %u bytes)\n", 
                 msgType, length);
    return false;
}

// ============================================================
// Debug
// ============================================================

void LoRaProtocol::printMessage(const uint8_t* data, size_t length) {
    if (!data || length < 1) {
        Serial.println("[LoRa] Empty message");
        return;
    }
    
    uint8_t msgType = getMessageType(data, length);
    
    Serial.printf("[LoRa] Message Type: 0x%02X, Length: %u bytes\n", msgType, length);
    
    // Print based on type
    switch (msgType) {
        case MSG_TYPE_TELEMETRY: {
            BeaconTelemetryMsg msg;
            if (parseMessage(data, length, &msg)) {
                Serial.printf("  Beacon ID: %u\n", msg.beaconId);
                Serial.printf("  Timestamp: %lu ms\n", msg.timestamp);
                Serial.printf("  Battery: %u%%\n", msg.batteryPercent);
                Serial.printf("  RSSI GW1-4: %d, %d, %d, %d dBm\n", 
                             msg.rssiGateway1, msg.rssiGateway2, 
                             msg.rssiGateway3, msg.rssiGateway4);
                Serial.printf("  Accel: X=%d, Y=%d, Z=%d mg\n", 
                             msg.accelX, msg.accelY, msg.accelZ);
                if (msg.lastCheckpointId > 0) {
                    Serial.printf("  Last Checkpoint: %u at %lu ms\n", 
                                 msg.lastCheckpointId, msg.checkpointTimestamp);
                }
            }
            break;
        }
        
        case MSG_TYPE_CHECKPOINT: {
            BeaconCheckpointMsg msg;
            if (parseMessage(data, length, &msg)) {
                Serial.printf("  Beacon ID: %u\n", msg.beaconId);
                Serial.printf("  Checkpoint ID: %u\n", msg.checkpointId);
                Serial.printf("  UUID: %.6s\n", msg.checkpointUUID);
                Serial.printf("  RSSI: %d dBm\n", msg.rssi);
            }
            break;
        }
        
        case MSG_TYPE_CRASH: {
            BeaconCrashMsg msg;
            if (parseMessage(data, length, &msg)) {
                Serial.printf("  Beacon ID: %u\n", msg.beaconId);
                Serial.printf("  Impact: X=%d, Y=%d, Z=%d mg\n", 
                             msg.impactAccelX, msg.impactAccelY, msg.impactAccelZ);
            }
            break;
        }
        
        case MSG_TYPE_COMMAND: {
            BeaconCommandMsg msg;
            if (parseMessage(data, length, &msg)) {
                Serial.printf("  Target Beacon: %u\n", msg.targetBeaconId);
                Serial.printf("  Command: 0x%02X\n", msg.command);
                Serial.printf("  Parameter: %lu\n", msg.param);
            }
            break;
        }
        
        case MSG_TYPE_DISPLAY_CMD: {
            DisplayCommandMsg msg;
            if (parseMessage(data, length, &msg)) {
                Serial.printf("  Display ID: %u\n", msg.displayId);
                Serial.printf("  Mode: 0x%02X\n", msg.mode);
                Serial.printf("  Value: %u\n", msg.value);
            }
            break;
        }
        
        default:
            // Hex dump
            Serial.print("  Raw: ");
            for (size_t i = 0; i < length; i++) {
                Serial.printf("%02X ", data[i]);
            }
            Serial.println();
            break;
    }
}



