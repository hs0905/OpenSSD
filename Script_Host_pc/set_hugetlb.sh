#!/bin/bash
export LIBHUGEFS_DIR=/usr/lib
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_overcommit_hugepages'
sudo sh -c 'echo "1024" >  /proc/sys/vm/nr_hugepages'