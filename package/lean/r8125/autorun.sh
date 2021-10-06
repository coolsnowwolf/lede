#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only

# invoke insmod with all arguments we got
# and use a pathname, as insmod doesn't look in . by default

TARGET_PATH=$(find /lib/modules/$(uname -r)/kernel/drivers/net/ethernet -name realtek -type d)
if [ "$TARGET_PATH" = "" ]; then
	TARGET_PATH=$(find /lib/modules/$(uname -r)/kernel/drivers/net -name realtek -type d)
fi
if [ "$TARGET_PATH" = "" ]; then
	TARGET_PATH=/lib/modules/$(uname -r)/kernel/drivers/net
fi
echo
echo "Check old driver and unload it."
check=`lsmod | grep r8169`
if [ "$check" != "" ]; then
        echo "rmmod r8169"
        /sbin/rmmod r8169
fi

check=`lsmod | grep r8125`
if [ "$check" != "" ]; then
        echo "rmmod r8125"
        /sbin/rmmod r8125
fi

echo "Build the module and install"
echo "-------------------------------" >> log.txt
date 1>>log.txt
make $@ all 1>>log.txt || exit 1
module=`ls src/*.ko`
module=${module#src/}
module=${module%.ko}

if [ "$module" = "" ]; then
	echo "No driver exists!!!"
	exit 1
elif [ "$module" != "r8169" ]; then
	if test -e $TARGET_PATH/r8169.ko ; then
		echo "Backup r8169.ko"
		if test -e $TARGET_PATH/r8169.bak ; then
			i=0
			while test -e $TARGET_PATH/r8169.bak$i
			do
				i=$(($i+1))
			done
			echo "rename r8169.ko to r8169.bak$i"
			mv $TARGET_PATH/r8169.ko $TARGET_PATH/r8169.bak$i
		else
			echo "rename r8169.ko to r8169.bak"
			mv $TARGET_PATH/r8169.ko $TARGET_PATH/r8169.bak
		fi
	fi
fi

echo "DEPMOD $(uname -r)"
depmod `uname -r`
echo "load module $module"
modprobe $module

is_update_initramfs=n
distrib_list="ubuntu debian"

if [ -r /etc/debian_version ]; then
	is_update_initramfs=y
elif [ -r /etc/lsb-release ]; then
	for distrib in $distrib_list
	do
		/bin/grep -i "$distrib" /etc/lsb-release 2>&1 /dev/null && \
			is_update_initramfs=y && break
	done
fi

if [ "$is_update_initramfs" = "y" ]; then
	if which update-initramfs >/dev/null ; then
		echo "Updating initramfs. Please wait."
		update-initramfs -u -k $(uname -r)
	else
		echo "update-initramfs: command not found"
		exit 1
	fi
fi

echo "Completed."
exit 0

