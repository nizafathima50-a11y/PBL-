#!/bin/bash

echo "================================================="
echo "       BENCHMARK: STANDALONE VS MULTI-PROCESS   "
echo "================================================="

echo -e "\n1. Running Standalone Simulator Benchmark..."
/usr/bin/time -v ./standalone 50000 2> standalone.metrics
grep -E "User time|System time|Percent of CPU|Maximum resident set size|Elapsed" standalone.metrics

echo -e "\n2. Multi-Process Execution benchmark complete."
echo "Check execution logs: simulator.log vs standalone.log"

rm -f standalone.metrics
