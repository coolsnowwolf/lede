#!/bin/sh
#
# Copyright (C) 2011-2012 OpenWrt.org
#

ubootenv_add_uci_config() {
	local dev=$1
	local offset=$2
	local envsize=$3
	local secsize=$4
	local numsec=$5
	uci batch <<EOF
add ubootenv ubootenv
set ubootenv.@ubootenv[-1].dev='$dev'
set ubootenv.@ubootenv[-1].offset='$offset'
set ubootenv.@ubootenv[-1].envsize='$envsize'
set ubootenv.@ubootenv[-1].secsize='$secsize'
set ubootenv.@ubootenv[-1].numsec='$numsec'
EOF
	uci commit ubootenv
}

ubootenv_add_app_config() {
	local dev
	local offset
	local envsize
	local secsize
	local numsec
	config_get dev "$1" dev
	config_get offset "$1" offset
	config_get envsize "$1" envsize
	config_get secsize "$1" secsize
	config_get numsec "$1" numsec
	grep -q "^[[:space:]]*${dev}[[:space:]]*${offset}" /etc/fw_env.config || echo "$dev $offset $envsize $secsize $numsec" >>/etc/fw_env.config
}

