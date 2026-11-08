#ifndef LAP_DETECTOR_H
#define LAP_DETECTOR_H

// Reine C++-Logik ohne Arduino-Abhaengigkeit (laeuft auch nativ fuer Tests/Replay).
//
// Pro Beacon/Team ein Detektor. Eine "Durchfahrt" beginnt, wenn der geglaettete
// RSSI >= nearRssi ist, und endet wenn er < farRssi faellt ODER fuer
// lostTimeoutMs kein Paket mehr kam. Zeitstempel der Runde = Zeitpunkt des
// staerksten (geglaetteten) Signals der Durchfahrt.

#include <stdint.h>

struct LapDetectorConfig {
    int8_t   nearRssi      = -65;    // ab hier "nah"
    int8_t   farRssi       = -80;    // darunter "weg"
    uint32_t minLapMs      = 10000;  // Mindestabstand zwischen zwei Runden
    uint32_t lostTimeoutMs = 2000;   // keine Pakete -> Durchfahrt beendet
    float    smoothing     = 0.5f;   // EMA-Faktor (1.0 = keine Glaettung)
};

class LapDetector {
public:
    enum State : uint8_t { AWAY = 0, NEAR = 1 };

    explicit LapDetector(const LapDetectorConfig& cfg = LapDetectorConfig());

    void setConfig(const LapDetectorConfig& cfg) { cfg_ = cfg; }
    void reset();

    // Neues Paket. true = Runde abgeschlossen (lapTs = Zeitpunkt des Signalmaximums).
    bool onSample(uint32_t ts, int8_t rssi, uint32_t& lapTs);

    // Regelmaessig aufrufen (Beacon verschwunden). true = Runde abgeschlossen.
    bool tick(uint32_t now, uint32_t& lapTs);

    State   state() const        { return state_; }
    float   smoothedRssi() const { return ema_; }
    uint32_t lastSampleTs() const { return lastSampleTs_; }
    uint32_t sampleCount() const { return samples_; }

private:
    bool finishPass(uint32_t& lapTs);

    LapDetectorConfig cfg_;
    State    state_;
    float    ema_;
    bool     haveEma_;
    float    peakRssi_;
    uint32_t peakTs_;
    uint32_t lastSampleTs_;
    uint32_t lastLapTs_;
    bool     haveLap_;
    uint32_t samples_;
};

#endif
