#!/bin/bash

of=( /mnt/lustre/vhs/tempfile.out /disk0/vhs/tempfile.out /dev/shm/tempfile.out )
out=disk_benchmarks.out

for i in {1..10} 
do
    for o in ${of[@]}
    do
        ./bench_disks.sh $o $out
    done
done
