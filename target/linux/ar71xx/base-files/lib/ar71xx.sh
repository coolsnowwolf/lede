#!/bin/sh
#
# Copyright (C) 2009-2011 OpenWrt.org
#

AR71XX_BOARD_NAME=
AR71XX_MODEL=

ar71xx_get_mtd_offset_size_format() {
	local mtd="$1"
	local offset="$2"
	local size="$3"
	local format="$4"
	local dev

	dev=$(find_mtd_part $mtd)
	[ -z "$dev" ] && return

	dd if=$dev bs=1 skip=$offset count=$size 2>/dev/null | hexdump -v -e "1/1 \"$format\""
}

ar71xx_get_mtd_part_magic() {
	local mtd="$1"
	ar71xx_get_mtd_offset_size_format "$mtd" 0 4 %02x
}

wndr3700_board_detect() {
	local machine="$1"
	local magic
	local name

	name="wndr3700"

	magic="$(ar71xx_get_mtd_part_magic firmware)"
	case $magic in
	"33373030")
		machine="NETGEAR WNDR3700"
		;;
	"33373031")
		model="$(ar71xx_get_mtd_offset_size_format art 41 32 %c)"
		# Use awk to remove everything unprintable
		model_stripped="$(ar71xx_get_mtd_offset_size_format art 41 32 %c | LC_CTYPE=C awk -v 'FS=[^[:print:]]' '{print $1; exit}')"
		case $model in
		$'\xff'*)
			if [ "${model:24:1}" = 'N' ]; then
				machine="NETGEAR WNDRMAC"
			else
				machine="NETGEAR WNDR3700v2"
			fi
			;;
		'29763654+16+64'*)
			machine="NETGEAR ${model_stripped:14}"
			;;
		'29763654+16+128'*)
			machine="NETGEAR ${model_stripped:15}"
			;;
		*)
			# Unknown ID
			machine="NETGEAR ${model_stripped}"
		esac
	esac

	AR71XX_BOARD_NAME="$name"
	AR71XX_MODEL="$machine"
}

ubnt_get_mtd_part_magic() {
	ar71xx_get_mtd_offset_size_format EEPROM 4118 2 %02x
}

ubnt_xm_board_detect() {
	local model
	local magic

	magic="$(ubnt_get_mtd_part_magic)"
	case ${magic:0:3} in
	"e00"|\
	"e01"|\
	"e80")
		model="Ubiquiti NanoStation M"
		;;
	"e0a")
		model="Ubiquiti NanoStation loco M"
		;;
	"e1b"|\
	"e1d")
		model="Ubiquiti Rocket M"
		;;
	"e20"|\
	"e2d")
		model="Ubiquiti Bullet M"
		;;
	"e30")
		model="Ubiquiti PicoStation M"
		;;
	esac

	[ -z "$model" ] || AR71XX_MODEL="${model}${magic:3:1}"
}

ubnt_ac_lite_get_mtd_part_magic() {
	ar71xx_get_mtd_offset_size_format EEPROM 12 2 %02x
}

ubnt_ac_lite_board_detect() {
	local model
	local magic

	magic="$(ubnt_ac_lite_get_mtd_part_magic)"
	case ${magic:0:4} in
	"e517")
		model="Ubiquiti UniFi-AC-LITE"
		;;
	"e557")
		model="Ubiquiti UniFi-AC-MESH"
		;;
	esac

	[ -z "$model" ] || AR71XX_MODEL="${model}"
}

cybertan_get_hw_magic() {
	local part

	part=$(find_mtd_part firmware)
	[ -z "$part" ] && return 1

	dd bs=8 count=1 skip=0 if=$part 2>/dev/null | hexdump -v -n 8 -e '1/1 "%02x"'
}

dir505_board_detect() {
	local dev=$(find_mtd_part 'mac')
	[ -z "$dev" ] && return

	# The revision is stored at the beginning of the "mac" partition
	local rev="$(LC_CTYPE=C awk -v 'FS=[^[:print:]]' '{print $1; exit}' $dev)"
	AR71XX_MODEL="D-Link DIR-505 rev. $rev"
}

tplink_get_hwid() {
	local part

	part=$(find_mtd_part firmware)
	[ -z "$part" ] && return 1

	dd if=$part bs=4 count=1 skip=16 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_get_mid() {
	local part

	part=$(find_mtd_part firmware)
	[ -z "$part" ] && return 1

	dd if=$part bs=4 count=1 skip=17 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_board_detect() {
	local model="$1"
	local hwid
	local hwver

	hwid=$(tplink_get_hwid)
	mid=$(tplink_get_mid)
	hwver=${hwid:6:2}
	hwver=" v${hwver#0}"

	case "$hwid" in
	"001001"*)
		model="TP-Link TL-MR10U"
		;;
	"001101"*)
		model="TP-Link TL-MR11U"
		;;
	"001201"*)
		model="TP-Link TL-MR12U"
		;;
	"001301"*)
		model="TP-Link TL-MR13U"
		;;
	"007260"*)
		model="TellStick ZNet Lite"
		;;
	"015000"*)
		model="EasyLink EL-M150"
		;;
	"015300"*)
		model="EasyLink EL-MINI"
		;;
	"044401"*)
		model="ANTMINER-S1"
		;;
	"044403"*)
		model="ANTMINER-S3"
		;;
	"066601"*)
		model="OMYlink OMY-G1"
		;;
	"066602"*)
		model="OMYlink OMY-X1"
		;;
	"070100"*)
		model="TP-Link TL-WA701N/ND"
		;;
	"070301"*)
		model="TP-Link TL-WR703N"
		;;
	"071000"*)
		model="TP-Link TL-WR710N"

		[ "$hwid" = '07100002' -a "$mid" = '00000002' ] && hwver=' v2.1'
		;;
	"072001"*)
		model="TP-Link TL-WR720N"
		;;
	"073000"*)
		model="TP-Link TL-WA730RE"
		;;
	"074000"*)
		model="TP-Link TL-WR740N/ND"
		;;
	"074100"*)
		model="TP-Link TL-WR741N/ND"
		;;
	"074300"*)
		model="TP-Link TL-WR743N/ND"
		;;
	"075000"*)
		model="TP-Link TL-WA750RE"
		;;
	"080100"*)
		model="TP-Link TL-WA801N/ND"
		;;
	"080200"*)
		model="TP-Link TL-WR802N"

		[ "$hwid" = '08020002' -a "$mid" = '00000002' ] && hwver=' v2'
		;;
	"081000"*)
		model="TP-Link TL-WR810N"
		;;
	"083000"*)
		model="TP-Link TL-WA830RE"

		[ "$hwver" = ' v10' ] && hwver=' v1'
		;;
	"084100"*)
		model="TP-Link TL-WR841N/ND"

		[ "$hwid" = '08410002' -a "$mid" = '00000002' ] && hwver=' v1.5'
		;;
	"084200"*)
		model="TP-Link TL-WR842N/ND"
		;;
	"084300"*)
		model="TP-Link TL-WR843N/ND"
		;;
	"085000"*)
		model="TP-Link TL-WA850RE"
		;;
	"085500"*)
		model="TP-Link TL-WA855RE"
		;;
	"086000"*)
		model="TP-Link TL-WA860RE"
		;;
	"090100"*)
		model="TP-Link TL-WA901N/ND"
		;;
	"094000"*)
		model="TP-Link TL-WR940N"
		;;
	"094100"*)
		model="TP-Link TL-WR941N/ND"

		[ "$hwid" = "09410002" -a "$mid" = "00420001" ] && {
			model="Rosewill RNX-N360RT"
			hwver=""
		}
		;;
	"104100"*)
		model="TP-Link TL-WR1041N/ND"
		;;
	"104300"*)
		model="TP-Link TL-WR1043N/ND"
		;;
	"120000"*)
		model="MERCURY MAC1200R"
		;;
	"254300"*)
		model="TP-Link TL-WR2543N/ND"
		;;
	"302000"*)
		model="TP-Link TL-MR3020"
		;;
	"304000"*)
		model="TP-Link TL-MR3040"
		;;
	"322000"*)
		model="TP-Link TL-MR3220"
		;;
	"332000"*)
		model="TP-Link TL-WDR3320"
		;;
	"342000"*)
		model="TP-Link TL-MR3420"
		;;
	"350000"*)
		model="TP-Link TL-WDR3500"
		;;
	"360000"*)
		model="TP-Link TL-WDR3600"
		;;
	"430000"*)
		model="TP-Link TL-WDR4300"
		;;
	"430080"*)
		iw reg set IL
		model="TP-Link TL-WDR4300 (IL)"
		;;
	"431000"*)
		model="TP-Link TL-WDR4310"
		;;
	"44440101"*)
		model="ANTROUTER-R1"
		;;
	"453000"*)
		model="Mercury MW4530R"
		;;
	"49000002")
		model="TP-Link TL-WDR4900"
		;;
	"640000"*)
		model="TP-Link TL-MR6400"
		;;
	"65000002")
		model="TP-Link TL-WDR6500"
		;;
	"721000"*)
		model="TP-Link TL-WA7210N"
		;;
	"750000"*|\
	"c70000"*)
		model="TP-Link Archer C7"
		;;
	"751000"*)
		model="TP-Link TL-WA7510N"
		;;
	"934100"*)
		model="NC-LINK SMART-300"
		;;
	"c50000"*)
		model="TP-Link Archer C5"
		;;
	*)
		hwver=""
		;;
	esac

	AR71XX_MODEL="$model$hwver"
}

tplink_pharos_get_model_string() {
	local part
	part=$(find_mtd_part 'product-info')
	[ -z "$part" ] && return 1

	# The returned string will end with \r\n, but we don't remove it here
	# to simplify matching against it in the sysupgrade image check
	dd if=$part bs=1 skip=4360 2>/dev/null | head -n 1
}

tplink_pharos_board_detect() {
	local model_string="$1"
	local oIFS="$IFS"; IFS=":"; set -- $model_string; IFS="$oIFS"

	local model="${1%%\(*}"

	AR71XX_MODEL="TP-Link $model v$2"
}

tplink_pharos_v2_get_model_string() {
	local part
	part=$(find_mtd_part 'product-info')
	[ -z "$part" ] && return 1

	dd if=$part bs=1 skip=4360 count=64 2>/dev/null | tr -d '\r\0' | head -n 1
}

ar71xx_board_detect() {
	local machine
	local name

	machine=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /machine/ {print $2}' /proc/cpuinfo)

	case "$machine" in
	*"A40")
		name="a40"
		;;
	*"A60")
		name="a60"
		;;
	*"AC1750DB")
		name="f9k1115v2"
		;;
	*"AirGateway")
		name="airgateway"
		;;
	*"AirGateway Pro")
		name="airgatewaypro"
		;;
	*"AirRouter")
		name="airrouter"
		;;
	*"ALFA Network AP120C")
		name="alfa-ap120c"
		;;
	*"ALFA Network AP96")
		name="alfa-ap96"
		;;
	*"ALFA Network N2/N5")
		name="alfa-nx"
		;;
	*"ALL0258N")
		name="all0258n"
		;;
	*"ALL0305")
		name="all0305"
		;;
	*"ALL0315N")
		name="all0315n"
		;;
	*"Antminer-S1")
		name="antminer-s1"
		;;
	*"Antminer-S3")
		name="antminer-s3"
		;;
	*"AP121 reference board")
		name="ap121"
		;;
	*"AP121-MINI")
		name="ap121-mini"
		;;
	*"AP121F")
		name="ap121f"
		;;
	*"AP132 reference board")
		name="ap132"
		;;
	*"AP135-020 reference board")
		name="ap135-020"
		;;
	*"AP136-010 reference board")
		name="ap136-010"
		;;
	*"AP136-020 reference board")
		name="ap136-020"
		;;
	*"AP143 reference board")
		name="ap143"
		;;
	*"AP147-010 reference board")
		name="ap147-010"
		;;
	*"AP152 reference board")
		name="ap152"
		;;
	*"AP531B0")
		name="ap531b0"
		;;
	*"AP90Q")
		name="ap90q"
		;;
	*"AP91-5G")
		name="ap91-5g"
		;;
	*"Archer C25 v1")
		name="archer-c25-v1"
		;;
	*"Archer C5")
		name="archer-c5"
		;;
	*"Archer C7 v4")
		name="archer-c7-v4"
		;;
	*"Archer C7 v5")
		name="archer-c7-v5"
		;;
	*"Archer C58 v1")
		name="archer-c58-v1"
		;;
	*"Archer C59 v1")
		name="archer-c59-v1"
        ;;
	*"Archer C59 v2")
		name="archer-c59-v2"
		;;
	*"Archer C60 v1")
		name="archer-c60-v1"
		;;
	*"Archer C60 v2")
		name="archer-c60-v2"
		;;
	*"Archer C7")
		name="archer-c7"
		;;
	*"Arduino Yun")
		name="arduino-yun"
		;;
	*"Atheros AP96")
		name="ap96"
		;;
	*"AW-NR580")
		name="aw-nr580"
		;;
	*"BHR-4GRV2")
		name="bhr-4grv2"
		;;
	*"Black Swift board"*)
		name="bsb"
		;;
	*"Bullet M")
		name="bullet-m"
		ubnt_xm_board_detect
		;;
	*"BXU2000n-2 rev. A1")
		name="bxu2000n-2-a1"
		;;
	*"C-55")
		name="c-55"
		;;
	*"C-60")
		name="c-60"
		;;
	*"CAP324")
		name="cap324"
		;;
	*"CAP4200AG")
		name="cap4200ag"
		;;
	*"Carambola2"*)
		name="carambola2"
		;;
	*"CF-E316N v2")
		name="cf-e316n-v2"
		;;
	*"CF-E320N v2")
		name="cf-e320n-v2"
		;;
	*"CF-E355AC v1")
		name="cf-e355ac-v1"
		;;
	*"CF-E355AC v2")
		name="cf-e355ac-v2"
		;;
	*"CF-E375AC")
		name="cf-e375ac"
		;;
	*"CF-E380AC v1")
		name="cf-e380ac-v1"
		;;
	*"CF-E380AC v2")
		name="cf-e380ac-v2"
		;;
	*"CF-E385AC")
		name="cf-e385ac"
		;;
	*"CF-E520N")
		name="cf-e520n"
		;;
	*"CF-E530N")
		name="cf-e530n"
		;;
	*"CPE210/220")
		name="cpe210"
		tplink_pharos_board_detect "$(tplink_pharos_get_model_string | tr -d '\r')"
		;;
	*"CPE210 v2")
		name="cpe210-v2"
		tplink_pharos_board_detect "$(tplink_pharos_v2_get_model_string)"
		;;
	*"CPE505N")
		name="cpe505n"
		;;
	*"CPE510/520")
		name="cpe510"
		tplink_pharos_board_detect "$(tplink_pharos_v2_get_model_string)"
		case $AR71XX_MODEL in
		'TP-Link CPE510 v2.0')
			;;
		*)
			tplink_pharos_board_detect "$(tplink_pharos_get_model_string | tr -d '\r')"
			;;
		esac
		;;
	*"CPE830")
		name="cpe830"
		;;
	*"CPE870")
		name="cpe870"
		;;
	*"CR3000")
		name="cr3000"
		;;
	*"CR5000")
		name="cr5000"
		;;
	*"DAP-1330 Rev. A1")
		name="dap-1330-a1"
		;;
	*"DAP-2695 rev. A1")
		name="dap-2695-a1"
		;;
	*"DB120 reference board")
		name="db120"
		;;
	*"DGL-5500 rev. A1")
		name="dgl-5500-a1"
		;;
	*"DHP-1565 rev. A1")
		name="dhp-1565-a1"
		;;
	*"DIR-505 rev. A1")
		name="dir-505-a1"
		dir505_board_detect
		;;
	*"DIR-600 rev. A1")
		name="dir-600-a1"
		;;
	*"DIR-615 rev. C1")
		name="dir-615-c1"
		;;
	*"DIR-615 rev. E1")
		name="dir-615-e1"
		;;
	*"DIR-615 rev. E4")
		name="dir-615-e4"
		;;
	*"DIR-615 rev. I1")
		name="dir-615-i1"
		;;
	*"DIR-825 rev. B1")
		name="dir-825-b1"
		;;
	*"DIR-825 rev. C1")
		name="dir-825-c1"
		;;
	*"DIR-835 rev. A1")
		name="dir-835-a1"
		;;
	*"DIR-869 rev. A1")
		name="dir-869-a1"
		;;
	*"dLAN Hotspot")
		name="dlan-hotspot"
		;;
	*"dLAN pro 1200+ WiFi ac")
		name="dlan-pro-1200-ac"
		;;
	*"dLAN pro 500 Wireless+")
		name="dlan-pro-500-wp"
		;;
	*"Domino Pi")
		name="gl-domino"
		;;
	*"DR342")
		name="dr342"
		;;
	*"DR344")
		name="dr344"
		;;
	*"DR531")
		name="dr531"
		;;
	*"Dragino v2")
		name="dragino2"
		;;
	*"DW33D")
		name="dw33d"
		;;
	*"E1700AC v2")
		name="e1700ac-v2"
		;;
	*"E2100L")
		name="e2100l"
		;;
	*"E558 v2")
		name="e558-v2"
		;;
	*"E600G v2")
		name="e600g-v2"
		;;
	*"E600GAC v2")
		name="e600gac-v2"
		;;
	*"E750A v4")
		name="e750a-v4"
		;;
	*"E750G v8")
		name="e750g-v8"
		;;
	*"EAP120")
		name="eap120"
		tplink_pharos_board_detect "$(tplink_pharos_get_model_string | tr -d '\r')"
		;;
	*"EAP300 v2")
		name="eap300v2"
		;;
	*"EAP7660D")
		name="eap7660d"
		;;
	*"EBR-2310 rev. C1")
		name="ebr-2310-c1"
		;;
	*"EL-M150")
		name="el-m150"
		;;
	*"EL-MINI")
		name="el-mini"
		;;
	*"EmbWir-Balin")
		name="ew-balin"
		;;
	*"EmbWir-Dorin")
		name="ew-dorin"
		;;
	*"EmbWir-Dorin-Router")
		name="ew-dorin-router"
		;;
	*"ENS202EXT")
		name="ens202ext"
		;;
	*"EPG5000")
		name="epg5000"
		;;
	*"ESR1750")
		name="esr1750"
		;;
	*"ESR900")
		name="esr900"
		;;
	*"eTactica EG-200")
		name="rme-eg200"
		;;
	*"FRITZ!Box 4020")
		name="fritz4020"
		;;
	*"FRITZ!WLAN Repeater 300E")
		name="fritz300e"
		;;
	*"FRITZ!WLAN Repeater 450E")
		name="fritz450e"
		;;
	*"GL-AR150")
		name="gl-ar150"
		;;
	*"GL-AR300")
		name="gl-ar300"
		;;
	*"GL-AR300M")
		name="gl-ar300m"
		;;
	*"GL-AR750")
		name="gl-ar750"
		;;
	*"GL-AR750S")
		name="gl-ar750s"
		;;
	*"GL-CONNECT INET v1")
		name="gl-inet"

		local size="$(mtd_get_part_size 'firmware')"

		[ "$size" = "8192000" ] && AR71XX_MODEL="GL-iNet 6408A v1"
		[ "$size" = "16580608" ] && AR71XX_MODEL="GL-iNet 6416A v1"
		;;
	*"GL-MIFI")
		name="gl-mifi"
		;;
	*"GL-USB150")
		name="gl-usb150"
		;;
	*"HiveAP-121")
		name="hiveap-121"
		;;
	*"HiWiFi HC6361")
		name="hiwifi-hc6361"
		;;
	*"Hornet-UB")
		local size="$(mtd_get_part_size 'firmware')"

		[ "$size" = "7929856" ] && name="hornet-ub"
		[ "$size" = "16318464" ] && name="hornet-ub-x2"
		;;
	*"JA76PF")
		name="ja76pf"
		;;
	*"JA76PF2")
		name="ja76pf2"
		;;
	*"JWAP003")
		name="jwap003"
		;;
	*"JWAP230")
		name="jwap230"
		;;
	*"Koala")
		name="koala"
		;;
	*"LAN Turtle")
		name="lan-turtle"
		;;
	*"Lima"*)
		name="lima"
		;;
	*"Litebeam M5"*)
		name="lbe-m5"
		;;
	*"Loco M XW")
		name="loco-m-xw"
		;;
	*"LS-SR71")
		name="ls-sr71"
		;;
	*"MAC1200R")
		name="mc-mac1200r"
		;;
	*"MiniBox V1.0")
		name="minibox-v1"
		;;
	*"MR12")
		name="mr12"
		;;
	*"MR16")
		name="mr16"
		;;
	*"MR1750")
		name="mr1750"
		;;
	*"MR1750v2")
		name="mr1750v2"
		;;
	*"MR18")
		name="mr18"
		;;
	*"MR600")
		name="mr600"
		;;
	*"MR600v2")
		name="mr600v2"
		;;
	*"MR900")
		name="mr900"
		;;
	*"MR900v2")
		name="mr900v2"
		;;
	*"My Net N600")
		name="mynet-n600"
		;;
	*"My Net N750")
		name="mynet-n750"
		;;
	*"My Net Wi-Fi Range Extender")
		name="mynet-rext"
		;;
	*"MZK-W04NU")
		name="mzk-w04nu"
		;;
	*"MZK-W300NH")
		name="mzk-w300nh"
		;;
	*"N5Q")
		name="n5q"
		;;
	*"Nanostation M")
		name="nanostation-m"
		ubnt_xm_board_detect
		;;
	*"Nanostation M XW")
		name="nanostation-m-xw"
		;;
	*"NBG460N/550N/550NH")
		name="nbg460n_550n_550nh"
		;;
	*"NBG6616")
		name="nbg6616"
		;;
	*"NBG6716")
		name="nbg6716"
		;;
	*"OM2P")
		name="om2p"
		;;
	*"OM2P HS")
		name="om2p-hs"
		;;
	*"OM2P HSv2")
		name="om2p-hsv2"
		;;
	*"OM2P HSv3")
		name="om2p-hsv3"
		;;
	*"OM2P HSv4")
		name="om2p-hsv4"
		;;
	*"OM2P LC")
		name="om2p-lc"
		;;
	*"OM2Pv2")
		name="om2pv2"
		;;
	*"OM2Pv4")
		name="om2pv4"
		;;
	*"OM5P")
		name="om5p"
		;;
	*"OM5P AC")
		name="om5p-ac"
		;;
	*"OM5P ACv2")
		name="om5p-acv2"
		;;
	*"OM5P AN")
		name="om5p-an"
		;;
	*"OMY-G1")
		name="omy-g1"
		;;
	*"OMY-X1")
		name="omy-x1"
		;;
	*"Onion Omega")
		name="onion-omega"
		;;
	*"Oolite V1.0")
		name="oolite-v1"
		;;
	*"Packet Squirrel")
		name="packet-squirrel"
		;;
	*"Oolite V5.2")
		name="oolite-v5.2"
		;;
	*"Oolite V5.2-Dev")
		name="oolite-v5.2-dev"
		;;
	*"PB42")
		name="pb42"
		;;
	*"PB44 reference board")
		name="pb44"
		;;
	*"PQI Air Pen")
		name="pqi-air-pen"
		;;
	*"Qihoo 360 C301")
		name="qihoo-c301"
		;;
	*"R36A")
		name="r36a"
		;;
	*"R602N")
		name="r602n"
		;;
	*"R6100")
		name="r6100"
		;;
	*"Rambutan"*)
		name="rambutan"
		;;
	*"RE355")
		name="re355"
		;;
	*"RE450")
		name="re450"
		;;
	*"Rocket M")
		name="rocket-m"
		ubnt_xm_board_detect
		;;
	*"Rocket M TI")
		name="rocket-m-ti"
		;;
	*"Rocket M XW")
		name="rocket-m-xw"
		;;
	*"RouterBOARD 2011iL")
		name="rb-2011il"
		;;
	*"RouterBOARD 2011iLS")
		name="rb-2011ils"
		;;
	*"RouterBOARD 2011L")
		name="rb-2011l"
		;;
	*"RouterBOARD 2011UAS")
		name="rb-2011uas"
		;;
	*"RouterBOARD 2011UAS-2HnD")
		name="rb-2011uas-2hnd"
		;;
	*"RouterBOARD 2011UiAS")
		name="rb-2011uias"
		;;
	*"RouterBOARD 2011UiAS-2HnD")
		name="rb-2011uias-2hnd"
		;;
	*"RouterBOARD 411/A/AH")
		name="rb-411"
		;;
	*"RouterBOARD 411U")
		name="rb-411u"
		;;
	*"RouterBOARD 433/AH")
		name="rb-433"
		;;
	*"RouterBOARD 433UAH")
		name="rb-433u"
		;;
	*"RouterBOARD 435G")
		name="rb-435g"
		;;
	*"RouterBOARD 450")
		name="rb-450"
		;;
	*"RouterBOARD 450G")
		name="rb-450g"
		;;
	*"RouterBOARD 493/AH")
		name="rb-493"
		;;
	*"RouterBOARD 493G")
		name="rb-493g"
		;;
	*"RouterBOARD 750")
		name="rb-750"
		;;
	*"RouterBOARD 750 r2")
		name="rb-750-r2"
		;;
	*"RouterBOARD 750GL")
		name="rb-750gl"
		;;
	*"RouterBOARD 750P r2")
		name="rb-750p-pbr2"
		;;
	*"RouterBOARD 750UP r2")
		name="rb-750up-r2"
		;;
	*"RouterBOARD 751")
		name="rb-751"
		;;
	*"RouterBOARD 751G")
		name="rb-751g"
		;;
	*"RouterBOARD 911-2Hn")
		name="rb-911-2hn"
		;;
	*"RouterBOARD 911-5Hn")
		name="rb-911-5hn"
		;;
	*"RouterBOARD 911G-2HPnD")
		name="rb-911g-2hpnd"
		;;
	*"RouterBOARD 911G-5HPacD")
		name="rb-911g-5hpacd"
		;;
	*"RouterBOARD 911G-5HPnD")
		name="rb-911g-5hpnd"
		;;
	*"RouterBOARD 912UAG-2HPnD")
		name="rb-912uag-2hpnd"
		;;
	*"RouterBOARD 912UAG-5HPnD")
		name="rb-912uag-5hpnd"
		;;
	*"RouterBOARD 921GS-5HPacD r2")
		name="rb-921gs-5hpacd-r2"
		;;
	*"RouterBOARD 931-2nD")
		name="rb-931-2nd"
		;;
	*"RouterBOARD 941-2nD")
		name="rb-941-2nd"
		;;
	*"RouterBOARD 951G-2HnD")
		name="rb-951g-2hnd"
		;;
	*"RouterBOARD 951Ui-2HnD")
		name="rb-951ui-2hnd"
		;;
	*"RouterBOARD 951Ui-2nD")
		name="rb-951ui-2nd"
		;;
	*"RouterBOARD 952Ui-5ac2nD")
		name="rb-952ui-5ac2nd"
		;;
	*"RouterBOARD 962UiGS-5HacT2HnT")
		name="rb-962uigs-5hact2hnt"
		;;
	*"RouterBOARD LHG 5nD")
		name="rb-lhg-5nd"
		;;
	*"RouterBOARD mAP 2nD")
		name="rb-map-2nd"
		;;
	*"RouterBOARD mAP L-2nD")
		name="rb-mapl-2nd"
		;;
	*"RouterBOARD SXT Lite2")
		name="rb-sxt2n"
		;;
	*"RouterBOARD SXT Lite5")
		name="rb-sxt5n"
		;;
	*"RouterBOARD wAP 2nD r2")
		name="rb-wap-2nd"
		;;
	*"RouterBOARD wAP R-2nD")
		name="rb-wapr-2nd"
		;;
	*"RouterBOARD wAP G-5HacT2HnD")
		name="rb-wapg-5hact2hnd"
		;;
	*"RouterStation")
		name="routerstation"
		;;
	*"RouterStation Pro")
		name="routerstation-pro"
		;;
	*"RUT900")
		name="rut900"
		;;
	*"RW2458N")
		name="rw2458n"
		;;
	*"SC1750")
		name="sc1750"
		;;
	*"SC300M")
		name="sc300m"
		;;
	*"SC450")
		name="sc450"
		;;
	*"SMART-300")
		name="smart-300"
		;;
	*"SOM9331")
		name="som9331"
		;;
	*"SR3200")
		name="sr3200"
		;;
	*"T830")
		name="t830"
		;;
	*"TellStick ZNet Lite")
		name="tellstick-znet-lite"
		;;
	*"TEW-632BRP")
		name="tew-632brp"
		;;
	*"TEW-673GRU")
		name="tew-673gru"
		;;
	*"TEW-712BR")
		name="tew-712br"
		;;
	*"TEW-732BR")
		name="tew-732br"
		;;
	*"TEW-823DRU")
		name="tew-823dru"
		;;
	*"TL-MR10U")
		name="tl-mr10u"
		;;
	*"TL-MR11U")
		name="tl-mr11u"
		;;
	*"TL-MR12U")
		name="tl-mr12u"
		;;
	*"TL-MR13U v1")
		name="tl-mr13u"
		;;
	*"TL-MR3020")
		name="tl-mr3020"
		;;
	*"TL-MR3040")
		name="tl-mr3040"
		;;
	*"TL-MR3040 v2")
		name="tl-mr3040-v2"
		;;
	*"TL-MR3220")
		name="tl-mr3220"
		;;
	*"TL-MR3220 v2")
		name="tl-mr3220-v2"
		;;
	*"TL-MR3420")
		name="tl-mr3420"
		;;
	*"TL-MR3420 v2")
		name="tl-mr3420-v2"
		;;
	*"TL-MR6400")
		name="tl-mr6400"
		;;
	*"TL-WA701ND v2")
		name="tl-wa701nd-v2"
		;;
	*"TL-WA7210N v2")
		name="tl-wa7210n-v2"
		;;
	*"TL-WA750RE")
		name="tl-wa750re"
		;;
	*"TL-WA7510N v1")
		name="tl-wa7510n"
		;;
	*"TL-WA801ND v2")
		name="tl-wa801nd-v2"
		;;
	*"TL-WA801ND v3")
		name="tl-wa801nd-v3"
		;;
	*"TL-WA830RE v2")
		name="tl-wa830re-v2"
		;;
	*"TL-WA850RE")
		name="tl-wa850re"
		;;
	*"TL-WA850RE v2")
		name="tl-wa850re-v2"
		;;
	*"TL-WA855RE v1")
		name="tl-wa855re-v1"
		;;
	*"TL-WA860RE")
		name="tl-wa860re"
		;;
	*"TL-WA901ND")
		name="tl-wa901nd"
		;;
	*"TL-WA901ND v2")
		name="tl-wa901nd-v2"
		;;
	*"TL-WA901ND v3")
		name="tl-wa901nd-v3"
		;;
	*"TL-WA901ND v4")
		name="tl-wa901nd-v4"
		;;
	*"TL-WA901ND v5")
		name="tl-wa901nd-v5"
		;;
	*"TL-WDR3320 v2")
		name="tl-wdr3320-v2"
		;;
	*"TL-WDR3500")
		name="tl-wdr3500"
		;;
	*"TL-WDR3600/4300/4310")
		name="tl-wdr4300"
		;;
	*"TL-WDR4900 v2")
		name="tl-wdr4900-v2"
		;;
	*"TL-WDR6500 v2")
		name="tl-wdr6500-v2"
		;;
	*"TL-WPA8630")
		name="tl-wpa8630"
		;;
	*"TL-WR1041N v2")
		name="tl-wr1041n-v2"
		;;
	*"TL-WR1043N v5")
		name="tl-wr1043n-v5"
		;;
	*"TL-WR1043ND")
		name="tl-wr1043nd"
		;;
	*"TL-WR1043ND v2")
		name="tl-wr1043nd-v2"
		;;
	*"TL-WR1043ND v4")
		name="tl-wr1043nd-v4"
		;;
	*"TL-WR2041N v1")
		name="tl-wr2041n-v1"
		;;
	*"TL-WR2041N v2")
		name="tl-wr2041n-v2"
		;;
	*"TL-WR2543N"*)
		name="tl-wr2543n"
		;;
	*"TL-WR703N v1")
		name="tl-wr703n"
		;;
	*"TL-WR710N v1")
		name="tl-wr710n"
		;;
	*"TL-WR720N"*)
		name="tl-wr720n-v3"
		;;
	*"TL-WR740N/ND v6")
		name="tl-wr740n-v6"
		;;
	*"TL-WR741ND")
		name="tl-wr741nd"
		;;
	*"TL-WR741ND v4")
		name="tl-wr741nd-v4"
		;;
	*"TL-WR802N v1")
		name="tl-wr802n-v1"
		;;
	*"TL-WR802N v2")
		name="tl-wr802n-v2"
		;;
	*"TL-WR810N")
		name="tl-wr810n"
		;;
	*"TL-WR810N v2")
		name="tl-wr810n-v2"
		;;
	*"TL-WR840N v2")
		name="tl-wr840n-v2"
		;;
	*"TL-WR840N v3")
		name="tl-wr840n-v3"
		;;
	*"TL-WR841N v1")
		name="tl-wr841n-v1"
		;;
	*"TL-WR841N/ND v11")
		name="tl-wr841n-v11"
		;;
	*"TL-WR841N/ND v7")
		name="tl-wr841n-v7"
		;;
	*"TL-WR841N/ND v8")
		name="tl-wr841n-v8"
		;;
	*"TL-WR841N/ND v9")
		name="tl-wr841n-v9"
		;;
	*"TL-WR842N/ND v2")
		name="tl-wr842n-v2"
		;;
	*"TL-WR842N/ND v3")
		name="tl-wr842n-v3"
		;;
	*"TL-WR880N v1")
		name="tl-wr880n-v1"
		;;
	*"TL-WR881N v1")
		name="tl-wr881n-v1"
		;;
	*"TL-WR902AC v1")
		name="tl-wr902ac-v1"
		;;
	*"TL-WR940N v4")
		name="tl-wr940n-v4"
		;;
	*"TL-WR940N v6")
		name="tl-wr940n-v6"
		;;
	*"TL-WR941N/ND v5")
		name="tl-wr941nd-v5"
		;;
	*"TL-WR941N/ND v6")
		name="tl-wr941nd-v6"
		;;
	*"TL-WR941N v7")
		name="tl-wr941n-v7"
		;;
	*"TL-WR941ND")
		name="tl-wr941nd"
		;;
	*"TL-WR942N v1")
		name="tl-wr942n-v1"
		;;
	*"TS-D084")
		name="ts-d084"
		;;
	*"Tube2H")
		name="tube2h"
		;;
	*"UniFi")
		name="unifi"
		;;
	*"UniFi AP Pro")
		name="uap-pro"
		;;
	*"UniFi-AC-LITE/MESH")
		name="unifiac-lite"
		ubnt_ac_lite_board_detect
		;;
	*"UniFi-AC-PRO")
		name="unifiac-pro"
		;;
	*"UniFiAP Outdoor")
		name="unifi-outdoor"
		;;
	*"UniFiAP Outdoor+")
		name="unifi-outdoor-plus"
		;;
	*"WAM250")
		name="wam250"
		;;
	*"WBS210")
		name="wbs210"
		tplink_pharos_board_detect "$(tplink_pharos_get_model_string | tr -d '\r')"
		;;
	*"WBS510")
		name="wbs510"
		tplink_pharos_board_detect "$(tplink_pharos_get_model_string | tr -d '\r')"
		;;
	"WeIO"*)
		name="weio"
		;;
	*"WI2A-AC200i")
		name="wi2a-ac200i"
		;;
	*"WHR-G301N")
		name="whr-g301n"
		;;
	*"WHR-HP-G300N")
		name="whr-hp-g300n"
		;;
	*"WHR-HP-GN")
		name="whr-hp-gn"
		;;
	*"WiFi Pineapple NANO")
		name="wifi-pineapple-nano"
		;;
	*"WLAE-AG300N")
		name="wlae-ag300n"
		;;
	*"WLR-8100")
		name="wlr8100"
		;;
	*"WNDAP360")
		name="wndap360"
		;;
	*"WNDR3700/WNDR3800/WNDRMAC")
		wndr3700_board_detect "$machine"
		;;
	*"WNDR3700v4")
		name="wndr3700v4"
		;;
	*"WNDR4300")
		name="wndr4300"
		;;
	*"WNR1000 V2")
		name="wnr1000-v2"
		;;
	*"WNR2000")
		name="wnr2000"
		;;
	*"WNR2000 V3")
		name="wnr2000-v3"
		;;
	*"WNR2000 V4")
		name="wnr2000-v4"
		;;
	*"WNR2200")
		name="wnr2200"
		;;
	*"WNR612 V2")
		name="wnr612-v2"
		;;
	*"WP543")
		name="wp543"
		;;
	*"WPE72")
		name="wpe72"
		;;
	*"WPJ342")
		name="wpj342"
		;;
	*"WPJ344")
		name="wpj344"
		;;
	*"WPJ531")
		name="wpj531"
		;;
	*"WPJ558")
		name="wpj558"
		;;
	*"WPJ563")
		name="wpj563"
		;;
	*"WPN824N")
		name="wpn824n"
		;;
	*"WRT160NL")
		name="wrt160nl"
		;;
	*"WRT400N")
		name="wrt400n"
		;;
	*"WRTnode2Q"*)
		name="wrtnode2q"
		;;
	*"WZR-450HP2")
		name="wzr-450hp2"
		;;
	*"WZR-HP-AG300H/WZR-600DHP")
		name="wzr-hp-ag300h"
		;;
	*"WZR-HP-G300NH")
		name="wzr-hp-g300nh"
		;;
	*"WZR-HP-G300NH2")
		name="wzr-hp-g300nh2"
		;;
	*"WZR-HP-G450H")
		name="wzr-hp-g450h"
		;;
	*"XD3200")
		name="xd3200"
		;;
	*"Z1")
		name="z1"
		;;
	*"ZBT-WE1526")
		name="zbt-we1526"
		;;
	*"ZCN-1523H-2")
		name="zcn-1523h-2"
		;;
	*"ZCN-1523H-5")
		name="zcn-1523h-5"
		;;
	esac

	[ -z "$AR71XX_MODEL" ] && [ "${machine:0:8}" = 'TP-LINK ' ] && \
		tplink_board_detect "$machine"

	[ -z "$name" ] && name="unknown"

	[ -z "$AR71XX_BOARD_NAME" ] && AR71XX_BOARD_NAME="$name"
	[ -z "$AR71XX_MODEL" ] && AR71XX_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$AR71XX_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$AR71XX_MODEL" > /tmp/sysinfo/model
}
