#include "LapDetector.h"

LapDetector::LapDetector(const LapDetectorConfig& cfg) : cfg_(cfg) { reset(); }

void LapDetector::reset() {
    state_ = AWAY;
    ema_ = 0;
    haveEma_ = false;
    peakRssi_ = -127;
    peakTs_ = 0;
    lastSampleTs_ = 0;
    lastLapTs_ = 0;
    haveLap_ = false;
    samples_ = 0;
}

bool LapDetector::finishPass(uint32_t& lapTs) {
    state_ = AWAY;
    haveEma_ = false;  // naechste Durchfahrt startet ohne Altlast
    uint32_t ts = peakTs_;
    peakRssi_ = -127;
    if (haveLap_ && (uint32_t)(ts - lastLapTs_) < cfg_.minLapMs) {
        return false;  // zu kurz nach der letzten Runde (Beacon "tanzt" an der Linie)
    }
    haveLap_ = true;
    lastLapTs_ = ts;
    lapTs = ts;
    return true;
}

bool LapDetector::onSample(uint32_t ts, int8_t rssi, uint32_t& lapTs) {
    bool lap = false;

    // Luecke > Timeout waehrend NEAR: alte Durchfahrt zuerst abschliessen
    if (state_ == NEAR && (uint32_t)(ts - lastSampleTs_) > cfg_.lostTimeoutMs) {
        lap = finishPass(lapTs);
    }

    ema_ = haveEma_ ? ema_ + cfg_.smoothing * ((float)rssi - ema_) : (float)rssi;
    haveEma_ = true;
    lastSampleTs_ = ts;
    samples_++;

    if (state_ == AWAY) {
        if (ema_ >= cfg_.nearRssi) {
            state_ = NEAR;
            peakRssi_ = ema_;
            peakTs_ = ts;
        }
    } else {
        if (ema_ > peakRssi_) {
            peakRssi_ = ema_;
            peakTs_ = ts;
        }
        if (ema_ < cfg_.farRssi) {
            uint32_t ts2 = 0;
            // Pro Aufruf wird max. eine Runde gemeldet (die aeltere hat Vorrang).
            if (finishPass(ts2) && !lap) {
                lapTs = ts2;
                lap = true;
            }
        }
    }
    return lap;
}

bool LapDetector::tick(uint32_t now, uint32_t& lapTs) {
    if (state_ == NEAR && (uint32_t)(now - lastSampleTs_) > cfg_.lostTimeoutMs) {
        return finishPass(lapTs);
    }
    return false;
}
