#!/bin/sh
#
# Copyright (C) 2014 OpenWrt.org
#

AT91_BOARD_NAME=
AT91_MODEL=

at91_board_detect() {
	local machine
	local name

	machine=$(cat /proc/device-tree/model)

	case "$machine" in
	*"Atmel at91sam9263ek")
		name="at91sam9263ek"
		;;
	*"Atmel AT91SAM9G15-EK")
		name="at91sam9g15ek"
		;;
	*"Atmel at91sam9g20ek")
		name="at91sam9g20ek"
		;;
	*"Atmel at91sam9g20ek 2 mmc")
		name="at91sam9g20ek_2mmc"
		;;
	*"Atmel AT91SAM9G25-EK")
		name="at91sam9g25ek"
		;;
	*"Atmel AT91SAM9G35-EK")
		name="at91sam9g35ek"
		;;
	*"Atmel AT91SAM9M10G45-EK")
		name="at91sam9m10g45ek"
		;;
	*"Atmel AT91SAM9X25-EK")
		name="at91sam9x25ek"
		;;
	*"Atmel AT91SAM9X35-EK")
		name="at91sam9x35ek"
		;;
	*"SAMA5D3 Xplained")
		name="sama5d3_xplained"
		;;
	*"Atmel SAMA5D27 SOM1 EK")
		name="sama5d27_som1_ek"
		;;
	*"Atmel SAMA5D2 PTC EK")
		name="sama5d2_ptc_ek"
		;;
	*"SAMA5D2 Xplained")
		name="sama5d2_xplained"
		;;
	*"SAMA5D4 Xplained")
		name="sama5d4_xplained"
		;;
	*"CalAmp LMU5000")
		name="lmu5000"
		;;
	*"Calao TNY A9260")
		name="tny_a9260"
		;;
	*"Calao TNY A9263")
		name="tny_a9263"
		;;
	*"Calao TNY A9G20")
		name="tny_a9g20"
		;;
	*"Calao USB A9260")
		name="usb_a9260"
		;;
	*"Calao USB A9263")
		name="usb_a9263"
		;;
	*"Calao USB A9G20")
		name="usb_a9g20"
		;;
	*"Ethernut 5")
		name="ethernut5"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$AT91_BOARD_NAME" ] && AT91_BOARD_NAME="$name"
	[ -z "$AT91_MODEL" ] && AT91_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$AT91_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$AT91_MODEL" > /tmp/sysinfo/model
}
