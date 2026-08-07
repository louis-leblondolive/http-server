#!/usr/bin/env bash
# Runs wrk at increasing concurrency levels against a given URL.
# Usage: ./wrk_sweep.sh http://127.0.0.1:3490/

URL="${1:?usage: bench.sh <url>}"
DURATION=10
CONCURRENCY=(1 100 500 1000 5000 10000 20000 30000)

for c in "${CONCURRENCY[@]}"; do
    threads=$((c < 4 ? c : 4))
    echo "=== concurrency=$c ==="
    wrk -t"$threads" -c"$c" -d"${DURATION}s" "$URL"
    echo
done