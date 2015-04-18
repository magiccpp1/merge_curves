#!/bin/sh
cd ..
tar -cvf merge.tar merge/merge.c merge/merge.h merge/README merge/data1.dat merge/data2.dat merge/tar.sh merge/Makefile
gzip merge.tar

