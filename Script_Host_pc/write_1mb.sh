#!/bin/bash
n=256
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
	let data_size=1048576 #4096
	let block_count=255 #0
        echo $t
        echo nvme write /dev/nvme0n1 --start-block=$num --data-size=$data_size --data=write_file/$t.bin --block-count=$block_count
        #sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=1048576 --data=/home/cpl/ariane/write_file/$t.bin --block-count=255 &
	sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=$data_size --data=/home/cpl/ariane/write_file/$t.bin --block-count=$block_count &
        pids[$v]=$!
    done

    for pid in ${pids[*]}; do
        wait $pid
    done
done
