#!/bin/bash
n=1
export LIBHUGEFS_DIR=/usr/lib
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_overcommit_hugepages'
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_hugepages'
for var in {0..63..1}
do
    for v in {0..15..1}
    do
        let a=16*${var}
        t=$(($v+a))
        let num=$(($(($v+a))*$n))
        counter=`expr 1000000 \+ $(($RANDOM*10)) \- 150000`
        bit=`expr $RANDOM \% 32`
        echo bit : $bit couter : $counter
	    sudo nvme io-passthru /dev/nvme0n1 --opcode=0x44 --cdw10=$1 --cdw11=$bit --cdw12=$counter #44
        echo $t
        echo nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=write_file/$t.bin --block-count=0
        sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=write_file/$t.bin --block-count=0 &
        pids[$v]=$!
    done

    for pid in ${pids[*]}; do
        wait $pid
    done
done
