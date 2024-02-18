#!/bin/bash
sudo nvme io-passthru /dev/nvme0n1 --opcode=0x43
