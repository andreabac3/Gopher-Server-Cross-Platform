#!/usr/bin/env bash
END=5000
for i in $(seq 1 $END); do curl gopher://localhost:9090/1/main.c &
pids[${i}]=$! ; done

# wait for all pids
for pid in ${pids[*]}; do
    wait $pid
done