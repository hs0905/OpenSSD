#!/bin/bash
n=1
echo reg: $1
sudo rm /home/cpl/ariane/read_file/*
for var in {0..1023..1}
do
        let a=$n*$var
	let data_size=1048576 #4096
	let block_count=255 #0
        echo $var
        # echo $a
        echo nvme read /dev/nvme0n1 --start-block=$a --data-size=$data_size --data=read_file/$var.bin --block-count=$block_count
        #counter=`expr 1000000 \+ $(($RANDOM*10)) \- 150000`
        #bit=`expr $RANDOM \% 32`

        #echo bit : $bit couter : $counter
	#sudo nvme io-passthru /dev/nvme0n1 --opcode=0x44 --cdw10=$1 --cdw11=$bit --cdw12=$counter #44
        sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme read /dev/nvme0n1 --start-block=$a --data-size=$data_size --data=/home/cpl/ariane/read_file/$var.bin --block-count=$block_count
        #sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme read /dev/nvme0n1 --start-block=$a --data-size=1048576 --data=/home/cpl/ariane/read_file/$var.bin --block-count=255

        diff /home/cpl/ariane/read_file/$var.bin /home/cpl/ariane/write_file/$var.bin
done
