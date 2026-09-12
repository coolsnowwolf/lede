#!/bin/sh

module="ftv310_vpu_driver"
name="ftv310_vpu"
device="/dev/dri/renderD128"
mode="666"

echo

#remove old nod
rm -f $device

#read the major asigned at loading time
major=`cat /proc/devices | grep $name | cut -c1-3`

echo "$name major = $major"

#create dev node
mknod $device c $major 0

echo "node $device created"

#give all 'rw' access
chmod $mode $device

echo "set node access to $mode"

#the end
echo
