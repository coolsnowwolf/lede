#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/duckbill
  NAME:=I2SE Duckbill boards
  FEATURES+=usbgadget
  PACKAGES+= \
	-dnsmasq -firewall -ppp -ip6tables -iptables -6relayd -mtd uboot-envtools \
	kmod-leds-gpio kmod-ledtrig-timer -kmod-ipt-nathelper uboot-mxs-duckbill
endef

define Profile/duckbill/Description
	I2SE's Duckbill devices
endef

$(eval $(call Profile,duckbill))
