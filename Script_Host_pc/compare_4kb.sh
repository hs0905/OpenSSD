#!/bin/bash
n=1
VAR=""
CNT=0
for var in {0..1023..1}
do     
        RESULT=$(cmp /home/cpl/ariane/read_file/$var.bin /home/cpl/ariane/write_4kb_file/$var.bin)
        if [ $RESULT -ne $VAR ];
        then
                CNT=$((CNT+1))
        fi
done
echo $((1024 - $CNT))
