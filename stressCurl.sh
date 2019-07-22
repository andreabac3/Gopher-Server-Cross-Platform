#!/usr/bin/env bash
END=5000
for i in $(seq 1 $END); do curl gopher://localhost:7070/1/README.md &
pids[${i}]=$! ; done

# wait for all pids
for pid in ${pids[*]}; do
    wait $pid
done