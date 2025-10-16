#!/bin/sh
MODDIR=${0%/*}
cd "$MODDIR" || exit

ADBEX_PATH=/data/adb/adbex
mkdir -p $ADBEX_PATH

if [ -f /linkerconfig/com.android.adbd/ld.config.txt ]; then
    echo "# adbex" >> /linkerconfig/com.android.adbd/ld.config.txt
    echo "namespace.default.permitted.paths += $ADBEX_PATH" >> /linkerconfig/com.android.adbd/ld.config.txt
fi

cp lib64/libadbex_init.so $ADBEX_PATH
cp lib64/libadbex_adbd.so $ADBEX_PATH
chcon -R u:object_r:system_file:s0 $ADBEX_PATH
./bin/inject 1 $ADBEX_PATH/libadbex_init.so
