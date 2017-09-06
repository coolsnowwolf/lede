#!/bin/sh
#
# Copyright (C) 2014 OpenWrt.org
#

KIRKWOOD_BOARD_NAME=
KIRKWOOD_MODEL=

kirkwood_board_detect() {
	local machine
	local name

	machine=$(cat /proc/device-tree/model)

	case "$machine" in
	"Seagate FreeAgent Dockstar")
		name="dockstar"
		;;

        "Seagate GoFlex Home")
                name="goflexhome"
                ;;

	"Seagate GoFlex Net")
		name="goflexnet"
		;;

	"Iomega Iconnect")
		name="iconnect"
		;;

	"RaidSonic ICY BOX IB-NAS62x0 (Rev B)")
		name="ib62x0"
		;;

	"Cisco Systems ON100")
		name="on100"
		;;

	"Cloud Engines Pogoplug E02")
		name="pogo_e02"
		;;

	"Linksys Audi (EA3500)")
		name="linksys-audi"
		;;

	"Linksys Viper (E4200v2 / EA4500)")
		name="linksys-viper"
		;;

	"Globalscale Technologies Guruplug Server Plus")
		name="guruplug-server-plus"
		;;

	"Globalscale Technologies SheevaPlug")
		name="sheevaplug"
		;;

	"Globalscale Technologies eSATA SheevaPlug")
		name="sheevaplug-esata"
		;;

	"ZyXEL NSA310b")
		name="nsa310b"
		;;

	"ZyXEL NSA310S")
		name="nsa310s"
		;;

	"ZyXEL NSA325")
		name="nsa325"
		;;

	*)
		name="generic"
		;;
	esac

	[ -z "$KIRKWOOD_BOARD_NAME" ] && KIRKWOOD_BOARD_NAME="$name"
	[ -z "$KIRKWOOD_MODEL" ] && KIRKWOOD_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$KIRKWOOD_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$KIRKWOOD_MODEL" > /tmp/sysinfo/model
}
