#!/bin/sh
dtype=`fdisk -l /dev/sda | grep 'Disklabel type' | awk '{print $3}'`
partid="0"

if [ "$dtype" = "gpt" ]
then
	partid=`echo "n



w
" | fdisk /dev/sda | grep 'Created a new partition' | awk '{print $5}'`

elif [ "$dtype" = "dos" ]
then
	partid=`echo "n
p



w
" | fdisk /dev/sda | grep 'Created a new partition' | awk '{print $5}'`
fi

echo "y" | mkfs.ext4 /dev/sda$partid
