#!/bin/sh

# kB disable if mem low than 256MB
memtotal=`grep MemTotal /proc/meminfo | awk '{print $2}'`
if test $memtotal -le 262144; then
	exit 0
fi
KZ=128
if test $memtotal -ge 8388608; then
	KZ=512
elif test $memtotal -ge 4194304; then
	KZ=256
fi

case $(uname -m) in
	i?86|x86_64)
		if ! grep -q crashkernel /boot/grub/grub.cfg; then
			mount /boot -o remount,rw
			sed -i "s/linux.*/& crashkernel=${KZ}M/" /boot/grub/grub.cfg
			mount /boot -o remount,ro
		fi
		;;
esac
