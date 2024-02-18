#!/bin/bash
n=256
export LIBHUGEFS_DIR=/usr/lib
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_overcommit_hugepages'
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_hugepages'
for var in {0..0..1}
do
    for v in {0..0..1}
    do
        let a=16*${var}
        t=$(($v+a))
        let num=$(($(($v+a))*$n))
        echo $t
        echo nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=write_file/$t.bin --block-count=0
        sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=write_file/$t.bin --block-count=0 &
        pids[$v]=$!
    done

    for pid in ${pids[*]}; do
        wait $pid
    done
done
