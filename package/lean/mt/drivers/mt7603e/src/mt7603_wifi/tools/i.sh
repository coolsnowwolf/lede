#!/bin/sh

SELF_PATH=$0
SELF_DIR=${SELF_PATH%\/*}
cd $SELF_DIR
TEST_DIR=$PWD
cd -

OS_DIR=$TEST_DIR/../os

#echo $OS_DIR
#ls $OS_DIR

insmod $OS_DIR/linux/mt7603e_ap.ko mac=00:11:22:33:44:55
ifconfig ra0 10.10.10.30 up
iwpriv ra0 set txcnt=65535

echo success