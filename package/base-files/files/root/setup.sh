#!/bin/sh

# THIS SCIPRT ONLY RUN ONCE. Base on /etc/firstboot_${board}

NEED_RESTART_SERVICE=0

setup_ssid()
{
    local r=$1
    local chip

    if ! uci show wireless.${r} >/dev/null 2>&1; then
        return
    fi

    logger "${TAG}: setup $1's ssid"
    wlan_path=/sys/devices/`uci get wireless.${r}.path`
    wlan_path=`find ${wlan_path} -name wlan* | tail -n 1`
    local mac=`cat ${wlan_path}/address`
    
    local dev_path=/sys/devices/`uci get wireless.${r}.path`

    if [ -e "${dev_path}/../idVendor" -a -e "${dev_path}/../idProduct" ]; then
	    idVendor=`cat ${dev_path}/../idVendor`
	    idProduct=`cat ${dev_path}/../idProduct`
	    if [ "x${idVendor}:${idProduct}" = "x0bda:c811" ]; then
		    chip="rtl8821cu"
		    touch ${FE_DIR}/first_insert_${chip}     # for /etc/hotplug.d/usb/31-usb_wifi
	    fi
    fi

    if [ -e "${dev_path}/vendor" -a -e "${dev_path}/device" ]; then
	    idVendor=`cat ${dev_path}/vendor`
	    idProduct=`cat ${dev_path}/device`

	    # enable 5g wifi-ap (t4)
	    if [ "x${idVendor}:${idProduct}" = "x0x02d0:0x4356" ]; then
		    uci set wireless.${r}.hwmode='11a'
		    uci set wireless.${r}.channel='153'
	    fi

	    # r2
            if [ "x${idVendor}:${idProduct}" = "x0x02d0:0xa9bf" ]; then
                    uci set wireless.${r}.hwmode='11a'
                    uci set wireless.${r}.channel='153'
            fi
    fi

    uci set wireless.${r}.disabled=0
    if [ -n "${chip}" ];then
        uci set wireless.default_${r}.ssid=${chip}-${mac}
    else
        uci set wireless.default_${r}.ssid=FriendlyWrt-${mac}
    fi
    uci set wireless.default_${r}.encryption=psk2
    uci set wireless.default_${r}.key=password
    uci commit
}

FE_DIR=/root/.friendlyelec/
mkdir -p ${FE_DIR}
TAG=friendlyelec
logger "${TAG}: /root/setup.sh running"

VENDOR=$(cat /tmp/sysinfo/board_name | cut -d , -f1)
BOARD=$(cat /tmp/sysinfo/board_name | cut -d , -f2)
if [ x${VENDOR} != x"friendlyelec" ]; then
	if [ x${VENDOR} != x"friendlyarm" ]; then
        	logger "only support friendlyelec boards. exiting..."
        	exit 0
	fi
fi

if [ -f /sys/class/sunxi_info/sys_info ]; then
    SUNXI_BOARD=`grep "board_name" /sys/class/sunxi_info/sys_info`
    SUNXI_BOARD=${SUNXI_BOARD#*FriendlyElec }

    logger "${TAG}: init for ${SUNXI_BOARD}"
    if ls /root/board/${SUNXI_BOARD}/* >/dev/null 2>&1; then
        cp -rf /root/board/${SUNXI_BOARD}/* /
    fi
fi

# update /etc/config/network
# WAN_IF=`uci get network.wan.ifname`
# if [ "x${WAN_IF}" = "xeth0" ]; then
# 	uci set network.wan.dns=8.8.8.8
# 	uci commit
# fi

WIFI_NUM=`find /sys/class/net/ -name wlan* | wc -l`
if [ ${WIFI_NUM} -gt 0 ]; then

    # make sure lan interface exist
    if [ -z "`uci get network.lan`" ]; then
        uci batch <<EOF
set network.lan='interface'
set network.lan.type='bridge'
set network.lan.proto='static'
set network.lan.ipaddr='192.168.2.1'
set network.lan.netmask='255.255.255.0'
set network.lan.ip6assign='60'
EOF
    fi
    
    # update /etc/config/wireless
    for i in `seq 0 ${WIFI_NUM}`; do
        setup_ssid radio${i}
    done
    NEED_RESTART_SERVICE=1
fi

if [ ${NEED_RESTART_SERVICE} -eq 1 ]; then
    /etc/init.d/led restart
    /etc/init.d/network restart
    /etc/init.d/dnsmasq restart
    logger "setup.sh: restart network services"
fi

# fix netdata issue
[ -d /usr/share/netdata/web ] && chown -R root:root /usr/share/netdata/web

# disable lan ipv6
#uci set 'network.lan.ipv6=off'
#uci set 'dhcp.lan.dhcpv6=disabled'
#/etc/init.d/odhcpd disable
#uci commit

logger "done"
