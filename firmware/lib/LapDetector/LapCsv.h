#ifndef LAP_CSV_H
#define LAP_CSV_H

// Parser fuer eine Zeile der Runden-CSV:
//   TeamID,Teamname,Runde,Timestamp(ms),Dauer(ms),HH:MM:SS
// Rein C++ (ohne Arduino) -> nativ testbar. Teamnamen duerfen Kommas enthalten
// (es wird von rechts geparst). Runde 0 = Start-Zeile.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct LapCsvRow {
    unsigned teamId;
    unsigned lap;
    uint32_t ts;
    uint32_t dur;
};

inline bool parseLapCsvLine(const char* line, LapCsvRow& out) {
    if (!line || line[0] < '0' || line[0] > '9') return false;  // Header/leer

    const char* firstComma = strchr(line, ',');
    if (!firstComma) return false;

    // Die letzten 4 Kommas von rechts finden: ...,lap,ts,dur,time
    const char* commas[4];
    const char* p = line + strlen(line);
    for (int i = 3; i >= 0; i--) {
        while (p > line && *(p - 1) != ',') p--;
        if (p <= line) return false;
        p--;                       // p zeigt auf das Komma
        commas[i] = p;
    }
    if (commas[0] <= firstComma) return false;  // Name/Felder fehlen

    out.teamId = (unsigned)strtoul(line, nullptr, 10);
    out.lap = (unsigned)strtoul(commas[0] + 1, nullptr, 10);
    out.ts = (uint32_t)strtoul(commas[1] + 1, nullptr, 10);
    out.dur = (uint32_t)strtoul(commas[2] + 1, nullptr, 10);
    return true;
}

#endif
