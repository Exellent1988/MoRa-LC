// Replay-Tool: spielt ein Roh-Log (ms,mac,rssi) der SD-Karte durch den echten LapDetector.
// Build:  g++ -std=c++11 -O2 -o replay tools/replay/replay.cpp lib/LapDetector/LapDetector.cpp
// Aufruf: ./replay races/xxx_raw.csv [--near -65] [--far -80] [--min-lap 10000]
//                                    [--timeout 2000] [--smooth 0.5]
#include "../../lib/LapDetector/LapDetector.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s raw.csv [--near N] [--far N] [--min-lap MS] [--timeout MS] [--smooth F]\n", argv[0]); return 2; }
    LapDetectorConfig cfg;
    for (int i = 2; i + 1 < argc; i += 2) {
        if (!strcmp(argv[i], "--near")) cfg.nearRssi = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "--far")) cfg.farRssi = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "--min-lap")) cfg.minLapMs = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "--timeout")) cfg.lostTimeoutMs = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "--smooth")) cfg.smoothing = atof(argv[i + 1]);
    }
    FILE* f = fopen(argv[1], "r");
    if (!f) { perror("open"); return 1; }

    std::map<std::string, LapDetector> det;
    std::map<std::string, int> laps, pkts;
    std::map<std::string, unsigned> lastLap;
    unsigned lastTs = 0;
    char line[128];
    fgets(line, sizeof line, f);  // Header
    while (fgets(line, sizeof line, f)) {
        unsigned ts; char mac[32]; int rssi;
        if (sscanf(line, "%u,%31[^,],%d", &ts, mac, &rssi) != 3) continue;
        std::string m(mac);
        if (!det.count(m)) det[m] = LapDetector(cfg);
        pkts[m]++;
        lastTs = ts;
        unsigned lapTs;
        if (det[m].onSample(ts, (int8_t)rssi, lapTs)) {
            laps[m]++;
            printf("%s lap %d at %u ms%s\n", mac, laps[m], lapTs,
                   laps[m] > 1 ? (" (+" + std::to_string(lapTs - lastLap[m]) + " ms)").c_str() : "");
            lastLap[m] = lapTs;
        }
        // Timeouts auch fuer alle anderen Beacons auswerten
        for (auto& kv : det) {
            unsigned t2;
            if (kv.first != m && kv.second.tick(ts, t2)) {
                laps[kv.first]++;
                printf("%s lap %d at %u ms (timeout)\n", kv.first.c_str(), laps[kv.first], t2);
                lastLap[kv.first] = t2;
            }
        }
    }
    for (auto& kv : det) {
        unsigned t2;
        if (kv.second.tick(lastTs + cfg.lostTimeoutMs + 1, t2)) { laps[kv.first]++; printf("%s lap %d at %u ms (end)\n", kv.first.c_str(), laps[kv.first], t2); }
    }
    printf("\n== Summary ==\n");
    for (auto& kv : pkts) printf("%s: %d packets, %d passes\n", kv.first.c_str(), kv.second, laps[kv.first]);
    fclose(f);
    return 0;
}
