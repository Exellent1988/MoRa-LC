#!/bin/sh
# Baut und startet die nativen Tests (kein ESP32 noetig).
set -e
cd "$(dirname "$0")/.."
OUT="${TMPDIR:-/tmp}/lap_detector_tests"
g++ -std=c++11 -Wall -Wextra -o "$OUT" test/test_lap_detector/test_main.cpp lib/LapDetector/LapDetector.cpp
"$OUT"
