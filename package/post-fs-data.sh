#!/bin/sh
MODDIR=${0%/*}
cd "$MODDIR" || exit

ADBEX_PATH=/sbin/adbex
[ -d /sbin ] || ADBEX_PATH=/debug_ramdisk/adbex

mkdir -p $ADBEX_PATH
cp lib64/libadbex_init.so $ADBEX_PATH
cp lib64/libadbex_adbd.so $ADBEX_PATH
chcon -R u:object_r:system_file:s0 $ADBEX_PATH
./bin/inject 1 $ADBEX_PATH/libadbex_init.so
