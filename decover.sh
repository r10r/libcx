#!/bin/sh
# http://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Gcov-Data-Files.html
# http://www.opensource.apple.com/source/gcc/gcc-5484/gcc/libgcov.c
# http://www.opensource.apple.com/source/gcc/gcc-1765/gcc/gcov-io.h
# https://chromium.googlesource.com/native_client/nacl-gcc/+/upstream/master/libgcc/libgcov-driver.c

echo 
echo "Coverage report (number of lines not covered)"
echo "============================================"

find . -name *.cov | xargs grep -c '#####' | awk -F: '($2 != 0)' | sort -r -n -k2 -t:
echo
