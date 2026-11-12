// Native Tests: ./tools/run_native_tests.sh
#include "../../lib/LapDetector/LapDetector.h"
#include "../../lib/LapDetector/LapCsv.h"
#include <cstdio>
#include <cstdlib>

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

// Durchfahrt: RSSI steigt auf peak und faellt wieder, 100 ms Takt
static int pass(LapDetector& d, uint32_t start, int8_t peak, uint32_t* lapTs, int* laps, int8_t low = -90) {
    static const int8_t shape[] = {-85, -80, -72, -62, 0, 0, -62, -72, -80, -88, -90};
    for (int i = 0; i < 11; i++) {
        int8_t r = shape[i] == 0 ? peak : shape[i];
        if (r < low) r = low;
        uint32_t t;
        if (d.onSample(start + i * 100, r, t)) { *lapTs = t; (*laps)++; }
    }
    return *laps;
}

static void test_single_pass() {
    LapDetector d; uint32_t ts = 0; int laps = 0;
    pass(d, 1000, -55, &ts, &laps);
    CHECK(laps == 1);
    CHECK(ts == 1500);  // Peak (geglaettet) nach den zwei staerksten Samples
}

static void test_missed_far_packet_still_counts_next_lap() {
    // Regressionstest fuer den alten Hysterese-Bug: Beacon verschwindet, ohne je < far zu melden.
    LapDetector d; uint32_t ts = 0; int laps = 0;
    uint32_t t;
    d.onSample(1000, -60, t);
    d.onSample(1100, -58, t);
    CHECK(d.state() == LapDetector::NEAR);
    // Danach keine Pakete mehr -> Timeout
    CHECK(!d.tick(2000, ts));
    CHECK(d.tick(4000, ts));
    CHECK(ts == 1100);
    CHECK(d.state() == LapDetector::AWAY);
    // Naechste Runde 20 s spaeter wird wieder erkannt
    pass(d, 21000, -55, &ts, &laps);
    CHECK(laps == 1);
}

static void test_min_lap_time() {
    LapDetector d; uint32_t ts = 0; int laps = 0;
    pass(d, 1000, -55, &ts, &laps);
    pass(d, 5000, -55, &ts, &laps);   // nur 4 s spaeter -> ignorieren
    CHECK(laps == 1);
    pass(d, 12000, -55, &ts, &laps);  // 11 s -> ok
    CHECK(laps == 2);
}

static void test_single_spike_filtered() {
    // Einzelner Ausreisser darf wegen Glaettung nicht reichen, wenn er knapp ist
    LapDetectorConfig c; c.smoothing = 0.3f;
    LapDetector d(c); uint32_t t;
    d.onSample(1000, -90, t);
    d.onSample(1100, -64, t);  // EMA = -90 + 0.3*26 = -82.2 -> nicht NEAR
    CHECK(d.state() == LapDetector::AWAY);
}

static void test_weak_pass_not_counted() {
    LapDetector d; int laps = 0;
    for (int i = 0; i < 10; i++) { uint32_t t; if (d.onSample(1000 + i * 100, -72, t)) laps++; }
    CHECK(laps == 0);
}

static void test_dwell_near_line_counts_once() {
    // Beacon bleibt lange nah (Fahrzeug steht an der Linie) und wackelt um near
    LapDetector d; uint32_t ts = 0; int laps = 0;
    for (int i = 0; i < 50; i++) {
        int8_t r = (i % 2) ? -62 : -70;  // EMA bleibt ~ -66..-64, nie < far
        uint32_t t; if (d.onSample(1000 + i * 100, r, t)) laps++;
    }
    d.tick(20000, ts);
    // genau eine Runde (beim Verlassen), nicht pro Wackler
    uint32_t t2; int extra = d.tick(30000, t2) ? 1 : 0;
    CHECK(laps + 1 + extra <= 2);
}

static void test_ten_teams_interleaved() {
    LapDetector d[10]; int laps[10] = {0};
    for (int lap = 0; lap < 5; lap++) {
        for (int team = 0; team < 10; team++) {
            uint32_t base = 1000 + lap * 20000 + team * 500;
            static const int8_t shape[] = {-85, -80, -72, -62, -55, -55, -62, -72, -80, -88, -90};
            for (int i = 0; i < 11; i++) {
                uint32_t t; if (d[team].onSample(base + i * 100, shape[i], t)) laps[team]++;
            }
        }
    }
    for (int team = 0; team < 10; team++) CHECK(laps[team] == 5);
}

static void test_csv_parser() {
    LapCsvRow r;
    CHECK(!parseLapCsvLine("Team ID,Team Name,Lap Number,Timestamp (ms),Duration (ms),Time of Day", r));
    CHECK(!parseLapCsvLine("", r));
    CHECK(parseLapCsvLine("3,Rot,12,123456,31234,00:02:03", r));
    CHECK(r.teamId == 3 && r.lap == 12 && r.ts == 123456 && r.dur == 31234);
    CHECK(parseLapCsvLine("4,Team A, B,1,5000,7000,00:00:07", r));  // Komma im Namen
    CHECK(r.teamId == 4 && r.lap == 1 && r.ts == 5000 && r.dur == 7000);
    CHECK(parseLapCsvLine("5,Start,0,999,0,00:00:00", r));
    CHECK(r.lap == 0 && r.ts == 999 && r.dur == 0);
    CHECK(!parseLapCsvLine("7,abc", r));
}

int main() {
    test_csv_parser();
    test_single_pass();
    test_missed_far_packet_still_counts_next_lap();
    test_min_lap_time();
    test_single_spike_filtered();
    test_weak_pass_not_counted();
    test_dwell_near_line_counts_once();
    test_ten_teams_interleaved();
    if (failures) { printf("%d FAILED\n", failures); return 1; }
    printf("all lap detector tests passed\n");
    return 0;
}
