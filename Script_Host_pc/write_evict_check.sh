#!/bin/bash
n=256
export LIBHUGEFS_DIR=/usr/lib
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_overcommit_hugepages'
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_hugepages'

sudo rm /home/cpl/ariane/read_file/*

for var in {0..1..1}
do
    for v in {0..8..1}
    do
        let a=16*${var}
        t=$(($v+a))
        let num=$(($(($v+a))*$n))
        echo $t
        echo nvme write /dev/nvme0n1 --start-block=$num --data-size=1048576 --data=/home/cpl/ariane/write_file/$t.bin --block-count=255
        sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=1048576 --data=/home/cpl/ariane/write_file/$t.bin --block-count=255 &
        pids[$v]=$!
    done

    for pid in ${pids[*]}; do
        wait $pid
    done
done


diff_num = 0
diff_num2 = 0
for varvar in {0..9..1}
do
    echo "eviction start"
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
            echo count_num :$t
            echo nvme write /dev/nvme0n1 --start-block=$num --data-size=1048576 --data=/home/cpl/ariane/write_file/$t.bin --block-count=255
            sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme write /dev/nvme0n1 --start-block=$num --data-size=1048576 --data=/home/cpl/ariane/write_file/$t.bin --block-count=255 &
            sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme read /dev/nvme0n1 --start-block=$a --data-size=1048576 --data=/home/cpl/ariane/read_file/$var.bin --block-count=255
            pids[$v]=$!
            
        done

        for pid in ${pids[*]}; do
            wait $pid
        done
    done


    for var in {0..1023..1}
    do
            let a=$n*$var
            echo $var
            sudo LD_PRELOAD=libhugetlbfs.so LD_LIBRARY_PATH=LIBHUGEFS_DIR:$LD_LIBRARY_PATH HUGETLB_MORECORE=yes nvme read /dev/nvme0n1 --start-block=$a --data-size=1048576 --data=/home/cpl/ariane/read_file/$var.bin --block-count=255
            let "diff_num2++"
            if [ -z "$(diff -BZ /home/cpl/ariane/read_file/$var.bin /home/cpl/ariane/write_file/$var.bin)"]; then
                echo "same"
            else
                let "diff_num++"
            fi
            #cmp /home/cpl/ariane/read_file/$var.bin /home/cpl/ariane/write_file/$var.bin
    done
done

echo diff_num : $diff_num
echo diff_num2 : $diff_num2
echo diff_end