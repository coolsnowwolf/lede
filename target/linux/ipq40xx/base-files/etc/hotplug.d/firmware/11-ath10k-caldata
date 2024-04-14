#!/bin/sh

[ -e /lib/firmware/$FIRMWARE ] && exit 0

. /lib/functions/caldata.sh

board=$(board_name)

case "$FIRMWARE" in
"ath10k/cal-pci-0000:01:00.0.bin")
	case "$board" in
	meraki,mr33)
		caldata_extract_ubi "ART" 0x9000 0x844
		caldata_valid "4408" || caldata_extract "ART" 0x9000 0x844
		ath10k_patch_mac $(macaddr_add $(get_mac_binary "/sys/bus/i2c/devices/0-0050/eeprom" 0x66) 1)
		;;
	esac
	;;
"ath10k/pre-cal-pci-0000:01:00.0.bin")
	case "$board" in
	asus,map-ac2200)
		caldata_extract_ubi "Factory" 0x9000 0x2f20
		ln -sf /lib/firmware/ath10k/pre-cal-pci-0000\:00\:00.0.bin \
			/lib/firmware/ath10k/QCA9888/hw2.0/board.bin
		;;
	asus,rt-ac42u)
		caldata_extract_ubi "Factory" 0x9000 0x2f20
		;;
	avm,fritzrepeater-3000)
		/usr/bin/fritz_cal_extract -i 1 -s 0x3D000 -e 0x212 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C800 -e 0x212 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C000 -e 0x212 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3D000 -e 0x212 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C800 -e 0x212 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C000 -e 0x212 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1")
		;;
	openmesh,a62 |\
	plasmacloud,pa2200)
		caldata_extract "0:ART" 0x9000 0x2f20
		;;
	linksys,ea8300 |\
	linksys,mr8300)
		caldata_extract "ART" 0x9000 0x2f20
		# OEM assigns 4 sequential MACs
		ath10k_patch_mac $(macaddr_setbit_la $(macaddr_add "$(cat /sys/class/net/eth0/address)" 4))
		;;
	netgear,rbr50|\
	netgear,rbs50|\
	netgear,srr60|\
	netgear,srs60)
		caldata_extract_mmc "0:ART" 0x9000 0x2f20
		ath10k_patch_mac $(mmc_get_mac_binary ARTMTD 0x12)
		;;
	esac
	;;
"ath10k/pre-cal-ahb-a000000.wifi.bin")
	case "$board" in
	qcom,ap-dk01.1-c1)
		caldata_extract "ART" 0x1000 0x2f20
		;;
	asus,map-ac2200|\
	asus,rt-ac42u|\
	asus,rt-ac58u)
		caldata_extract_ubi "Factory" 0x1000 0x2f20
		;;
	avm,fritzbox-4040)
		/usr/bin/fritz_cal_extract -i 1 -s 0x400 -e 0x207 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader_config")
		;;
	avm,fritzbox-7530 |\
	avm,fritzrepeater-1200 |\
	avm,fritzrepeater-3000)
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C000 -e 0x207 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C800 -e 0x207 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3D000 -e 0x207 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C000 -e 0x207 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C800 -e 0x207 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3D000 -e 0x207 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1")
		;;
	cellc,rtl30vw |\
	openmesh,a42 |\
	openmesh,a62 |\
	plasmacloud,pa1200 |\
	plasmacloud,pa2200)
		caldata_extract "0:ART" 0x1000 0x2f20
		;;
	devolo,magic-2-wifi-next)
		caldata_extract "ART" 0x1000 0x2f20
		ath10k_patch_mac $(mtd_get_mac_ascii APPSBLENV WiFiMacAddress0)
		;;
	dlink,dap-2610)
		caldata_extract "ART" 0x1000 0x2f20
		ath10k_patch_mac $(mtd_get_mac_ascii bdcfg wlanmac)
		;;
	engenius,emd1)
		caldata_extract "0:ART" 0x1000 0x2f20
		ath10k_patch_mac $(mtd_get_mac_ascii 0:APPSBLENV wlanaddr)
		;;
	engenius,emr3500)
		caldata_extract "0:ART" 0x1000 0x2f20
		ath10k_patch_mac $(mtd_get_mac_ascii 0:APPSBLENV ethaddr)
		;;
	engenius,ens620ext)
		caldata_extract "ART" 0x1000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii u-boot-env ethaddr) 2)
		;;
	linksys,ea8300 |\
	linksys,mr8300)
		caldata_extract "ART" 0x1000 0x2f20
		ath10k_patch_mac $(macaddr_add "$(cat /sys/class/net/eth0/address)" 2)
		;;
	meraki,mr33)
		caldata_extract_ubi "ART" 0x1000 0x2f20
		caldata_valid "202f" || caldata_extract "ART" 0x1000 0x2f20
		ath10k_patch_mac $(macaddr_add $(get_mac_binary "/sys/bus/i2c/devices/0-0050/eeprom" 0x66) 2)
		;;
	mikrotik,cap-ac |\
	mikrotik,hap-ac2 |\
	mikrotik,hap-ac3)
		wlan_data="/sys/firmware/mikrotik/hard_config/wlan_data"
		( [ -f "$wlan_data" ] && caldata_sysfsload_from_file "$wlan_data" 0x0 0x2f20 ) || \
		( [ -d "$wlan_data" ] && caldata_sysfsload_from_file "$wlan_data/data_0" 0x0 0x2f20 )
		;;
	netgear,rbr50|\
	netgear,rbs50|\
	netgear,srr60|\
	netgear,srs60)
		caldata_extract_mmc "0:ART" 0x1000 0x2f20
		ath10k_patch_mac $(mmc_get_mac_binary ARTMTD 0x0)
		;;
	teltonika,rutx10)
		caldata_extract "0:ART" 0x1000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_binary "0:CONFIG" 0x0) 2)
		;;
	zyxel,nbg6617 |\
	zyxel,wre6606)
		caldata_extract "ART" 0x1000 0x2f20
		ath10k_patch_mac $(macaddr_add $(cat /sys/class/net/eth0/address) -2)
		;;
	esac
	;;
"ath10k/pre-cal-ahb-a800000.wifi.bin")
	case "$board" in
	qcom,ap-dk01.1-c1)
		caldata_extract "ART" 0x5000 0x2f20
		;;
	asus,map-ac2200|\
	asus,rt-ac58u)
		caldata_extract_ubi "Factory" 0x5000 0x2f20
		;;
	avm,fritzbox-4040)
		/usr/bin/fritz_cal_extract -i 1 -s 0x400 -e 0x208 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader_config")
		;;
	avm,fritzbox-7530 |\
	avm,fritzrepeater-1200 |\
	avm,fritzrepeater-3000)
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C800 -e 0x208 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3D000 -e 0x208 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C000 -e 0x208 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader0") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C800 -e 0x208 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3D000 -e 0x208 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1") || \
		/usr/bin/fritz_cal_extract -i 1 -s 0x3C000 -e 0x208 -l 12064 -o /lib/firmware/$FIRMWARE $(find_mtd_chardev "urlader1")
		;;
	cellc,rtl30vw |\
	openmesh,a42 |\
	openmesh,a62 |\
	plasmacloud,pa1200 |\
	plasmacloud,pa2200)
		caldata_extract "0:ART" 0x5000 0x2f20
		;;
	devolo,magic-2-wifi-next)
		caldata_extract "ART" 0x5000 0x2f20
		ath10k_patch_mac $(mtd_get_mac_ascii APPSBLENV WiFiMacAddress1)
		;;
	dlink,dap-2610)
		caldata_extract "ART" 0x5000 0x2f20
		ath10k_patch_mac $(mtd_get_mac_ascii bdcfg wlanmac_a)
		;;
	engenius,emd1)
		caldata_extract "0:ART" 0x5000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii 0:APPSBLENV wlanaddr) 1)
		;;
	engenius,emr3500)
		caldata_extract "0:ART" 0x5000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii 0:APPSBLENV ethaddr) 1)
		;;
	engenius,ens620ext)
		caldata_extract "ART" 0x5000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_ascii u-boot-env ethaddr) 3)
		;;
	linksys,ea8300 |\
	linksys,mr8300)
		caldata_extract "ART" 0x5000 0x2f20
		ath10k_patch_mac $(macaddr_add "$(cat /sys/class/net/eth0/address)" 3)
		;;
	meraki,mr33)
		caldata_extract_ubi "ART" 0x5000 0x2f20
		caldata_valid "202f" || caldata_extract "ART" 0x5000 0x2f20
		ath10k_patch_mac $(macaddr_add $(get_mac_binary "/sys/bus/i2c/devices/0-0050/eeprom" 0x66) 3)
		;;
	mikrotik,cap-ac |\
	mikrotik,hap-ac2 |\
	mikrotik,hap-ac3 |\
	mikrotik,sxtsq-5-ac)
		wlan_data="/sys/firmware/mikrotik/hard_config/wlan_data"
		( [ -f "$wlan_data" ] && caldata_sysfsload_from_file "$wlan_data" 0x8000 0x2f20 ) || \
		( [ -d "$wlan_data" ] && caldata_sysfsload_from_file "$wlan_data/data_2" 0x0 0x2f20 )
		;;
	netgear,rbr50|\
	netgear,rbs50|\
	netgear,srr60|\
	netgear,srs60)
		caldata_extract_mmc "0:ART" 0x5000 0x2f20
		ath10k_patch_mac $(mmc_get_mac_binary ARTMTD 0xc)
		;;
	teltonika,rutx10)
		caldata_extract "0:ART" 0x5000 0x2f20
		ath10k_patch_mac $(macaddr_add $(mtd_get_mac_binary "0:CONFIG" 0x0) 3)
		;;
	zyxel,nbg6617 |\
	zyxel,wre6606)
		caldata_extract "ART" 0x5000 0x2f20
		ath10k_patch_mac $(macaddr_add $(cat /sys/class/net/eth0/address) -1)
		;;
	esac
	;;
"ath10k/QCA4019/hw1.0/board-ahb-a000000.wifi.bin")
	case "$board" in
	mikrotik,cap-ac |\
	mikrotik,hap-ac2 |\
	mikrotik,hap-ac3)
		wlan_data="/sys/firmware/mikrotik/hard_config/wlan_data"
		( [ -f "$wlan_data" ] && caldata_sysfsload_from_file "$wlan_data" 0x2f20 0x2f20 ) || \
		( [ -d "$wlan_data" ] && caldata_sysfsload_from_file "$wlan_data/data_0" 0x2f20 0x2f20 )
		;;
	esac
	;;
"ath10k/QCA4019/hw1.0/board-ahb-a800000.wifi.bin")
	case "$board" in
	mikrotik,cap-ac |\
	mikrotik,hap-ac2 |\
	mikrotik,hap-ac3 |\
	mikrotik,sxtsq-5-ac)
		wlan_data="/sys/firmware/mikrotik/hard_config/wlan_data"
		( [ -f "$wlan_data" ] && caldata_sysfsload_from_file "$wlan_data" 0xaf20 0x2f20 ) || \
		( [ -d "$wlan_data" ] && caldata_sysfsload_from_file "$wlan_data/data_2" 0x2f20 0x2f20 )
		;;
	esac
	;;
*)
	exit 1
	;;
esac
