#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

board=$(board_name)

case "$FIRMWARE" in
"ath10k/cal-pci-0000:01:00.0.bin")
	case "$board" in
	meraki,mr52)
		CI_UBIPART=art
		caldata_extract_ubi "ART" 0x1000 0x844
		;;
	ruijie,rg-mtfi-m520)
		caldata_extract "ART" 0x1000 0x844
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii PRODUCTINFO ethaddr) 2)
		;;
	esac
	;;
"ath10k/cal-pci-0001:01:00.0.bin")
	case "$board" in
	ruijie,rg-mtfi-m520)
		caldata_extract "ART" 0x5000 0x844
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii PRODUCTINFO ethaddr) 3)
		;;
	esac
	;;
"ath10k/pre-cal-pci-0000:01:00.0.bin")
	case $board in
	askey,rt4230w-rev6)
		caldata_extract "0:ART" 0x1000 0x2f20
		;;
	asrock,g10)
		caldata_extract "0:art" 0x1000 0x2f20
		;;
	linksys,ea7500-v1 |\
	linksys,ea8500)
		caldata_extract "art" 0x1000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii devinfo hw_mac_addr) 1)
		;;
	meraki,mr42)
		CI_UBIPART=art
		caldata_extract_ubi "ART" 0x1000 0x2f20
		;;
	zyxel,nbg6817)
		caldata_extract "0:art" 0x1000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii 0:appsblenv ethaddr) 1)
		;;
	esac
	;;
"ath10k/pre-cal-pci-0001:01:00.0.bin")
	case $board in
	askey,rt4230w-rev6)
		caldata_extract "0:ART" 0x5000 0x2f20
		;;
	asrock,g10)
		caldata_extract "0:art" 0x5000 0x2f20
		;;
	edgecore,ecw5410)
		caldata_extract "0:art" 0x1000 0x2f20
		;;
	linksys,ea7500-v1 |\
	linksys,ea8500)
		caldata_extract "art" 0x5000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii devinfo hw_mac_addr) 2)
		;;
	meraki,mr42 |\
	meraki,mr52)
		CI_UBIPART=art
		caldata_extract_ubi "ART" 0x5000 0x2f20
		;;
	zyxel,nbg6817)
		caldata_extract "0:art" 0x5000 0x2f20
		ath10k_patch_mac $(mtd_get_mac_ascii 0:appsblenv ethaddr)
		;;
	esac
	;;
"ath10k/cal-pci-0002:01:00.0.bin")
	case "$board" in
	meraki,mr42)
		CI_UBIPART=art
		caldata_extract_ubi "ART" 0x9000 0x844
		;;
	esac
	;;
"ath10k/pre-cal-pci-0002:01:00.0.bin")
	case $board in
	edgecore,ecw5410)
		caldata_extract "0:art" 0x5000 0x2f20
		;;
	meraki,mr52)
		CI_UBIPART=art
		caldata_extract_ubi "ART" 0x9000 0x2f20
		;;
	esac
	;;
*)
	exit 1
	;;
esac
