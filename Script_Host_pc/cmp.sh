#!/bin/bash
n=256
diff_num = 0
diff_num2 = 0

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

echo diff_num : $diff_num
echo diff_num2 : $diff_num2
echo diff_end