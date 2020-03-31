#!/bin/bash

enable_swap() {
    if [ -e /swapfile ]; then
        rm -f /swapfile
    fi

    DFRESULT=`df / | awk '$3 ~ /[0-9]+/ { print $4 }'`
    SWAPSIZE=1048576
    if [ $DFRESULT -gt 2500000 ]; then
        # 2G
        SWAPSIZE=2097152
    elif [ $DFRESULT -gt 2000000 ]; then
	# 1.5G
	SWAPSIZE=1572864
    elif [ $DFRESULT -gt 1500000 ]; then
        # 1G
	SWAPSIZE=1048576
    elif [ $DFRESULT -gt 800000 ]; then
	# 512M
        SWAPSIZE=524288
    else
        echo "Fail to enable swap: No space left on device (${DFRESULT})"
        exit 0
    fi
    
    dd if=/dev/zero of=/swapfile bs=1024 count=$SWAPSIZE
    chmod 600 /swapfile
    mkswap /swapfile
    swapon /swapfile

    sed -i '/^vm.swappiness=.*/d' /etc/sysctl.conf
    echo "vm.swappiness=10" >> /etc/sysctl.conf
    sysctl -p

    while uci -q delete fstab.@swap[-1]; do :; done
    uci add fstab swap
    uci set fstab.@swap[-1].enabled='1'
    uci set fstab.@swap[-1].device='/swapfile'
    uci set fstab.@swap[-1].label='foo'
    uci commit fstab
    echo "done"
}

HASSWAP=`swapon -s`
if ! grep swapfile ${HASSWAP} >/dev/null 2>&1; then
    enable_swap
else
    echo "swap has been enabled."
fi
