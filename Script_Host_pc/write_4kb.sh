#!/bin/bash
n=1
export LIBHUGEFS_DIR=/usr/lib
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_overcommit_hugepages'
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_hugepages'
for var in {0..63..1} #for var in {0..255..1}
do
  for v in {0..15..1}
    do
    	let a=16*var						#0 ~ 1008
			let t=v+a							#0 ~ 1023
			let num=t*n
      let data_size=4096 				#bytes
	    let block_count=0 				
      echo $t
      echo nvme write /dev/nvme0n1 --start-block=$num --data-size=$data_size --data=write_file/$t.bin --block-count=$block_count
	    sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=$data_size --data=/home/cpl/ariane/write_file/$t.bin --block-count=$block_count &
        pids[$v]=$!
    done

    for pid in ${pids[*]}; do
        wait $pid
    done
done
