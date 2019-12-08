#!/bin/sh
#
# Copyright (C) 2010-2013 OpenWrt.org
#

ramips_board_detect() {
	local machine
	local name

	machine=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /machine/ {print $2}' /proc/cpuinfo)

	case "$machine" in
	*"11AC NAS Router")
		name="11acnas"
		;;
	*"3G150B")
		name="3g150b"
		;;
	*"3G300M")
		name="3g300m"
		;;
	*"3g-6200n")
		name="3g-6200n"
		;;
	*"3g-6200nl")
		name="3g-6200nl"
		;;
	*"A5-V11")
		name="a5-v11"
		;;
	*"Ai-BR100")
		name="ai-br100"
		;;
	*"Air3GII")
		name="air3gii"
		;;
	*"ALL0256N (4M)")
		name="all0256n-4M"
		;;
	*"ALL0256N (8M)")
		name="all0256n-8M"
		;;
	*"ALL5002")
		name="all5002"
		;;
	*"ALL5003")
		name="all5003"
		;;
	*"AR670W")
		name="ar670w"
		;;
	*"AR725W")
		name="ar725w"
		;;
	*"ASL26555 (8M)")
		name="asl26555-8M"
		;;
	*"ASL26555 (16M)")
		name="asl26555-16M"
		;;
	*"ATP-52B")
		name="atp-52b"
		;;
	*"AWAPN2403")
		name="awapn2403"
		;;
	*"AWM002 EVB (4M)")
		name="awm002-evb-4M"
		;;
	*"AWM002 EVB (8M)")
		name="awm002-evb-8M"
		;;
	*"BC2")
		name="bc2"
		;;
	*"BOCCO")
		name="bocco"
		;;
	*"BR-6475nD")
		name="br-6475nd"
		;;
	*"Broadway")
		name="broadway"
		;;
	*"C108")
		name="c108"
		;;
	*"C20i")
		name="c20i"
		;;
	*"C50")
		name="c50"
		;;
	*"Carambola")
		name="carambola"
		;;
	*"CF-WR800N")
		name="cf-wr800n"
		;;
	*"CS-QR10")
		name="cs-qr10"
		;;
	*"CY-SWR1100")
		name="cy-swr1100"
		;;
	*"D105")
		name="d105"
		;;
	*"D240")
		name="d240"
		;;
	*"DAP-1350")
		name="dap-1350"
		;;
	*"DB-WRT01")
		name="db-wrt01"
		;;
	*"DCH-M225")
		name="dch-m225"
		;;
	*"DCS-930")
		name="dcs-930"
		;;
	*"DCS-930L B1")
		name="dcs-930l-b1"
		;;
	*"DIR-300 B1")
		name="dir-300-b1"
		;;
	*"DIR-300 B7")
		name="dir-300-b7"
		;;
	*"DIR-320 B1")
		name="dir-320-b1"
		;;
	*"DIR-600 B1")
		name="dir-600-b1"
		;;
	*"DIR-610 A1")
		name="dir-610-a1"
		;;
	*"DIR-615 D")
		name="dir-615-d"
		;;
	*"DIR-615 H1")
		name="dir-615-h1"
		;;
	*"DIR-620 A1")
		name="dir-620-a1"
		;;
	*"DIR-620 D1")
		name="dir-620-d1"
		;;
	*"DIR-645")
		name="dir-645"
		;;
	*"DIR-810L")
		name="dir-810l"
		;;
	*"DIR-860L B1")
		name="dir-860l-b1"
		;;
	*"Dovado Tiny AC")
		name="tiny-ac"
		;;
	*"DuZun DM06")
		name="duzun-dm06"
		;;
	*"DWR-512 B")
		name="dwr-512-b"
		;;
	*"E1700")
		name="e1700"
		;;
	*"ESR-9753")
		name="esr-9753"
		;;
	*"EW1200")
		name="ew1200"
		;;
	*"EX2700")
		name="ex2700";
		;;
	*"EX3700/EX3800")
		name="ex3700"
		;;
	*"F5D8235 v1")
		name="f5d8235-v1"
		;;
	*"F5D8235 v2")
		name="f5d8235-v2"
		;;
	*"F7C027")
		name="f7c027"
		;;
	*"FireWRT")
		name="firewrt"
		;;
	*"Fonera 2.0N")
		name="fonera20n"
		;;
	*"FreeStation5")
		name="freestation5"
		;;
	*"GL-MT300A")
		name="gl-mt300a"
		;;
	*"GL-MT300N")
		name="gl-mt300n"
		;;
	*"GL-MT750")
		name="gl-mt750"
		;;
	*"GL-MT300N-V2")
		name="gl-mt300n-v2"
		;;
	*"HC5661")
		name="hc5661"
		;;
	*"HC5661A")
		name="hc5661a"
		;;
	*"HC5761")
		name="hc5761"
		;;
	*"HC5861")
		name="hc5861"
		;;
	*"HC5962")
		name="hc5962"
		;;
	*"HG255D")
		name="hg255d"
		;;
	*"HLK-RM04")
		name="hlk-rm04"
		;;
	*"HPM")
		name="hpm"
		;;
	*"HT-TM02")
		name="ht-tm02"
		;;
	*"HW550-3G")
		name="hw550-3g"
		;;
	*"IP2202")
		name="ip2202"
		;;
	*"JHR-N805R")
		name="jhr-n805r"
		;;
	*"JHR-N825R")
		name="jhr-n825r"
		;;
	*"JHR-N926R")
		name="jhr-n926r"
		;;
	*"K2P")
		name="k2p"
		;;
	*"M3")
		name="m3"
		;;
	*"M4 (4M)")
		name="m4-4M"
		;;
	*"M4 (8M)")
		name="m4-8M"
		;;
	*"MediaTek LinkIt Smart 7688")
		name="linkits7688"
		;;
	*"Memory 2 Move")
		name="m2m"
		;;
	*"Mercury MAC1200R v2")
		name="mac1200rv2"
		;;
	*"Mi Router 3G")
		name="mir3g"
		;;
	*"Mi Router 4")
		name="mir4"
		;;		
	*"MicroWRT")
		name="microwrt"
		;;
	*"MiniEMBPlug")
		name="miniembplug"
		;;
	*"MiniEMBWiFi")
		name="miniembwifi"
		;;
	*"MiWiFi Mini")
		name="miwifi-mini"
		;;
	*"MiWiFi Nano")
		name="miwifi-nano"
		;;
	*"MLW221")
		name="mlw221"
		;;
	*"MLWG2")
		name="mlwg2"
		;;
	*"MOFI3500-3GN")
		name="mofi3500-3gn"
		;;
	*"MPR-A1")
		name="mpr-a1"
		;;
	*"MPR-A2")
		name="mpr-a2"
		;;
	*"MR-102N")
		name="mr-102n"
		;;
	*"MR200")
		name="mr200"
		;;
	*"MT7620a + MT7530 evaluation"*)
		name="mt7620a_mt7530"
		;;
	*"MT7620a V22SG"*)
		name="mt7620a_v22sg"
		;;
	*"MT7621 evaluation"*)
		name="mt7621"
		;;
	*"MT7628AN evaluation"*)
		name="mt7628"
		;;
	*"MT7688 evaluation"*)
		name="mt7688"
		;;
	*"MZK-750DHP")
		name="mzk-750dhp"
		;;
	*"MZK-DP150N")
		name="mzk-dp150n"
		;;
	*"MZK-EX300NP")
		name="mzk-ex300np"
		;;
	*"MZK-EX750NP")
		name="mzk-ex750np"
		;;
	*"MZK-W300NH2"*)
		name="mzk-w300nh2"
		;;
	*"MZK-WDPR"*)
		name="mzk-wdpr"
		;;
	*"NA930")
		name="na930"
		;;
	*"NBG-419N")
		name="nbg-419n"
		;;
	*"NBG-419N v2")
		name="nbg-419n2"
		;;
	*"Newifi-D1")
		name="newifi-d1"
		;;
	*"NCS601W")
		name="ncs601w"
		;;
	*"NixcoreX1 (8M)")
		name="nixcore-x1-8M"
		;;
	*"NixcoreX1 (16M)")
		name="nixcore-x1-16M"
		;;
	*"NW718")
		name="nw718"
		;;
	*"Onion Omega2")
		name="omega2"
		;;
	*"Onion Omega2+")
		name="omega2p"
		;;
	*"OY-0001")
		name="oy-0001"
		;;
	*"PBR-D1")
		name="pbr-d1"
		;;
	*"PBR-M1")
		name="pbr-m1"
		;;
	*"PSG1208")
		name="psg1208"
		;;
	*"PSG1218 rev.A")
		name="psg1218a"
		;;
	*"PSG1218 rev.B")
		name="psg1218b"
		;;
	*"PSR-680W"*)
		name="psr-680w"
		;;
	*"PWH2004")
		name="pwh2004"
		;;
	*"PX-4885 (4M)")
		name="px-4885-4M"
		;;
	*"PX-4885 (8M)")
		name="px-4885-8M"
		;;
	*"Q7")
		name="zte-q7"
		;;
	*"R6220")
		name="r6220"
		;;
	*"RB750Gr3")
		name="rb750gr3"
		;;
	*"RE350 v1")
		name="re350-v1"
		;;
	*"RE6500")
		name="re6500"
		;;
	*"RN502J")
		name="xdxrn502j"
		;;
	*"RP-N53")
		name="rp-n53"
		;;
	*"RT5350F-OLinuXino")
		name="rt5350f-olinuxino"
		;;
	*"RT5350F-OLinuXino-EVB")
		name="rt5350f-olinuxino-evb"
		;;
	*"RT-AC51U")
		name="rt-ac51u"
		;;
	*"RT-AC54U")
		name="rt-ac54u"
		;;
	*"RT-G32 B1")
		name="rt-g32-b1"
		;;
	*"RT-N10+")
		name="rt-n10-plus"
		;;
	*"RT-N12+")
		name="rt-n12p"
		;;
	*"RT-N13U")
		name="rt-n13u"
		;;
	*"RT-N14U")
		name="rt-n14u"
		;;
	*"RT-N15")
		name="rt-n15"
		;;
	*"RT-N56U")
		name="rt-n56u"
		;;
	*"RUT5XX")
		name="rut5xx"
		;;
	*"SamKnows Whitebox 8")
		name="sk-wb8"
		;;
	*"SAP-G3200U3")
		name="sap-g3200u3"
		;;
	*"SL-R7205"*)
		name="sl-r7205"
		;;
	*"TEW-638APB v2")
		name="tew-638apb-v2"
		;;
	*"TEW-691GR")
		name="tew-691gr"
		;;
	*"TEW-692GR")
		name="tew-692gr"
		;;
	*"TEW-714TRU")
		name="tew-714tru"
		;;
	*"Timecloud")
		name="timecloud"
		;;
	*"TL-WR840N v4")
		name="tl-wr840n-v4"
		;;
	*"TL-WR840N v5")
		name="tl-wr840n-v5"
		;;
	*"TL-WR841N v13")
		name="tl-wr841n-v13"
		;;
	*"U25AWF-H1")
		name="u25awf-h1"
		;;
	*"U7628-01 (128M RAM/16M flash)")
		name="u7628-01-128M-16M"
		;;
	*"UBNT-ERX")
		name="ubnt-erx"
		;;
	*"UBNT-ERX-SFP")
		name="ubnt-erx-sfp"
		;;
	*"UR-326N4G")
		name="ur-326n4g"
		;;
	*"UR-336UN")
		name="ur-336un"
		;;
	*"V11ST-FE")
		name="v11st-fe"
		;;
	*"V22RW-2X2")
		name="v22rw-2x2"
		;;
	*"VoCore (8M)")
		name="vocore-8M"
		;;
	*"VoCore (16M)")
		name="vocore-16M"
		;;
	*"VoCore2")
		name="vocore2"
		;;
	*"VoCore2-Lite")
		name="vocore2lite"
		;;
	*"VR500")
		name="vr500"
		;;
	*"W150M")
		name="w150m"
		;;
	*"W2914NS v2")
		name="w2914nsv2"
		;;
	*"W306R V2.0")
		name="w306r-v20"
		;;
	*"W502U")
		name="w502u"
		;;
	*"WCR-1166DS")
		name="wcr-1166ds"
		;;
	*"WCR-150GN")
		name="wcr-150gn"
		;;
	*"WE1026-5G (16M)")
		name="we1026-5g-16m"
		;;
	*"WF-2881")
		name="wf-2881"
		;;
	*"WHR-1166D")
		name="whr-1166d"
		;;
	*"WHR-300HP2")
		name="whr-300hp2"
		;;
	*"WHR-600D")
		name="whr-600d"
		;;
	*"WHR-G300N")
		name="whr-g300n"
		;;
	*"WIZARD 8800")
		name="wizard8800"
		;;
	*"WizFi630A")
		name="wizfi630a"
		;;
	*"WL-330N")
		name="wl-330n"
		;;
	*"WL-330N3G")
		name="wl-330n3g"
		;;
	*"WL-351 v1 002")
		name="wl-351"
		;;
	*"WL-WN575A3")
		name="wl-wn575a3"
		;;
	*"WLI-TX4-AG300N")
		name="wli-tx4-ag300n"
		;;
	*"WLR-6000")
		name="wlr-6000"
		;;
	*"WMDR-143N")
		name="wmdr-143n"
		;;
	*"WMR-300")
		name="wmr-300"
		;;
	*"WN3000RPv3")
		name="wn3000rpv3"
		;;
	*"WNCE2001")
		name="wnce2001"
		;;
	*"WNDR3700v5")
		name="wndr3700v5"
		;;
	*"WR512-3GN (4M)")
		name="wr512-3gn-4M"
		;;
	*"WR512-3GN (8M)")
		name="wr512-3gn-8M"
		;;
	*"WR6202")
		name="wr6202"
		;;
	*"WRH-300CR")
		name="wrh-300cr"
		;;
	*"WRTNODE")
		name="wrtnode"
		;;
	*"WRTnode2R")
		name="wrtnode2r"
		;;
	*"WRTnode2P")
		name="wrtnode2p"
		;;
	*"WSR-1166DHP")
		name="wsr-1166"
		;;
	*"WSR-600DHP")
		name="wsr-600"
		;;
	*"WT1520 (4M)")
		name="wt1520-4M"
		;;
	*"WT1520 (8M)")
		name="wt1520-8M"
		;;
	*"WT3020 (4M)")
		name="wt3020-4M"
		;;
	*"WT3020 (8M)")
		name="wt3020-8M"
		;;
	*"WZR-AGL300NH")
		name="wzr-agl300nh"
		;;
	*"X5")
		name="x5"
		;;
	*"X8")
		name="x8"
		;;
	*"Y1")
		name="y1"
		;;
	*"Y1S")
		name="y1s"
		;;
	*"ZBT-APE522II")
		name="zbt-ape522ii"
		;;
	*"ZBT-CPE102")
		name="zbt-cpe102"
		;;
	*"ZBT-WA05")
		name="zbt-wa05"
		;;
	*"ZBT-WE1326")
		name="zbt-we1326"
		;;
	*"ZBT-WE2026")
		name="zbt-we2026"
		;;
	*"ZBT-WE826 (16M)")
		name="zbt-we826-16M"
		;;
	*"ZBT-WE826 (32M)")
		name="zbt-we826-32M"
		;;
	*"ZBT-WG2626")
		name="zbt-wg2626"
		;;
	*"ZBT-WG3526 (16M)")
		name="zbt-wg3526-16M"
		;;
	*"ZBT-WG3526 (32M)")
		name="zbt-wg3526-32M"
		;;
	*"ZBT-WR8305RT")
		name="zbt-wr8305rt"
		;;
	*"ZyXEL Keenetic")
		name="kn"
		;;
	*"ZyXEL Keenetic Omni")
		name="kn_rc"
		;;
	*"ZyXEL Keenetic Omni II")
		name="kn_rf"
		;;
	*"ZyXEL Keenetic Viva")
		name="kng_rc"
		;;
	*"YK1")
		name="youku-yk1"
		;;
	esac

	# use generic board detect if no name is set
	[ -z "$name" ] && return

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$name" > /tmp/sysinfo/board_name
	echo "$machine" > /tmp/sysinfo/model
}
