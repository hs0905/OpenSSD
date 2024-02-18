#!/bin/bash
n=1
export LIBHUGEFS_DIR=/usr/lib
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_overcommit_hugepages'
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_hugepages'

sudo rm /home/cpl/ariane/read_file/*

for var in {0..127..1}
do
    for v in {0..15..1}
    do
        let a=16*${var}
        t=$(($(($v+a)) % 1024))
        # t=$(($v+a))
        let num=$(($(($v+a))*$n))
        echo $t
        #echo nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=/home/cpl/ariane/write_file/$t.bin --block-count=0
        echo nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=/home/cpl/ariane/write_file/$t.bin --block-count=0
        sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=/home/cpl/ariane/write_file/$t.bin --block-count=0 &
        pids[$v]=$!
    done

     for pid in ${pids[*]}; do
         wait $pid
     done
done


let diff_num = 0
let diff_num2 = 0
echo "error injection start..!"
for var in {0..0..1}
do
    for v in {0..1023..1} #original : 3, not 1024
    do
        let a=16*${var}
        t=$(($v+a))
        let num=$(($(($v+a))*$n))
        counter=`expr 1000000 \+ $(($RANDOM*10)) \- 150000`
	    #counter=887230
        bit=`expr $RANDOM \% 32`
	    #bit=8
        if [ $((v % 200)) -eq 0 ]; then
            echo bit : $bit couter : $counter
            sudo nvme io-passthru /dev/nvme0n1 --opcode=0x44 --cdw10=$1 --cdw11=$bit --cdw12=$counter #44
        fi
        echo count_num :$t
        echo nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=/home/cpl/ariane/write_file/$t.bin --block-count=0
        sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=4096 --data=/home/cpl/ariane/write_file/$t.bin --block-count=0 &
        pids[$vv]=$!

    done

    for pid in ${pids[*]}; do
        wait $pid
    done

done

#n_read=256
#for var in {0..1023..1}
#do
#        let a=$n_read*$var
#        echo $var
#        sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme read /dev/nvme0n1 --start-block=$a --data-size=1048576 --data=/home/cpl/ariane/read_file/$var.bin --block-count=255
#        let "diff_num2++"
#        if [ -z "$(diff -BZ /home/cpl/ariane/read_file/$var.bin /home/cpl/ariane/write_file/$var.bin)"]; then
#            echo "same"
#        else
#            let "diff_num++"
#        fi
        #cmp /home/cpl/ariane/read_file/$var.bin /home/cpl/ariane/write_file/$var.bin
#done

#echo diff_num : $diff_num
#echo diff_num2 : $diff_num2
#echo diff_end
