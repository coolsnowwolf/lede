#
# Copyright (C) 2010 OpenWrt.org
#

PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	alfa-network,quad-e4g)
		[ "$(fw_printenv -n dual_image 2>/dev/null)" = "1" ] &&\
		[ -n "$(find_mtd_part backup)" ] && {
			PART_NAME=backup
			if [ "$(fw_printenv -n bootactive 2>/dev/null)" = "1" ]; then
				fw_setenv bootactive 2 || exit 1
			else
				fw_setenv bootactive 1 || exit 1
			fi
		}
		;;
	ampedwireless,ally-00x19k|\
	ampedwireless,ally-r1900k)
		if [ "$(fw_printenv --lock / -n bootImage 2>/dev/null)" != "0" ]; then
			fw_setenv --lock / bootImage 0 || exit 1
		fi
		;;
	mikrotik,ltap-2hnd|\
	mikrotik,routerboard-750gr3|\
	mikrotik,routerboard-760igs|\
	mikrotik,routerboard-m11g|\
	mikrotik,routerboard-m33g)
		[ "$(rootfs_type)" = "tmpfs" ] && mtd erase firmware
		;;
	asus,rt-ac65p|\
	asus,rt-ac85p)
		echo "Backing up firmware"
		dd if=/dev/mtd4 bs=1024 count=4096  > /tmp/backup_firmware.bin
		dd if=/dev/mtd5 bs=1024 count=52224 >> /tmp/backup_firmware.bin
		mtd -e firmware2 write /tmp/backup_firmware.bin firmware2
		;;
	esac

	case "$board" in
	ampedwireless,ally-00x19k|\
	ampedwireless,ally-r1900k|\
	arcadyan,we420223-99|\
	asus,rt-ac65p|\
	asus,rt-ac85p|\
	asus,rt-ax53u|\
	asus,rt-ax54|\
	beeline,smartbox-flash|\
	beeline,smartbox-giga|\
	beeline,smartbox-turbo|\
	belkin,rt1800|\
	dlink,dap-x1860-a1|\
	dlink,dir-1960-a1|\
	dlink,dir-2640-a1|\
	dlink,dir-2660-a1|\
	dlink,dir-3060-a1|\
	dlink,dir-853-a3|\
	etisalat,s3|\
	h3c,tx1800-plus|\
	h3c,tx1801-plus|\
	h3c,tx1806|\
	haier,har-20s2u1|\
	hiwifi,hc5962|\
	iptime,a3004t|\
	iptime,ax2004m|\
	iptime,t5004|\
	jcg,q20|\
	linksys,e5600|\
	linksys,e7350|\
	linksys,ea6350-v4|\
	linksys,ea7300-v1|\
	linksys,ea7300-v2|\
	linksys,ea7500-v2|\
	linksys,ea8100-v1|\
	linksys,ea8100-v2|\
	mts,wg430223|\
	netgear,r6220|\
	netgear,r6260|\
	netgear,r6350|\
	netgear,r6700-v2|\
	netgear,r6800|\
	netgear,r6850|\
	netgear,r6900-v2|\
	netgear,r7200|\
	netgear,r7450|\
	netgear,wac104|\
	netgear,wac124|\
	netgear,wax202|\
	netis,wf2881|\
	raisecom,msg1500-x-00|\
	rostelecom,rt-sf-1|\
	sercomm,na502|\
	sercomm,na502s|\
	sim,simax1800t|\
	tplink,ec330-g5u-v1|\
	xiaomi,mi-router-3g|\
	xiaomi,mi-router-3-pro|\
	xiaomi,mi-router-4|\
	xiaomi,mi-router-ac2100|\
	xiaomi,mi-router-cr6606|\
	xiaomi,mi-router-cr6608|\
	xiaomi,mi-router-cr6609|\
	xiaomi,redmi-router-ac2100|\
	zyxel,nwa50ax|\
	zyxel,nwa55axe)
		nand_do_upgrade "$1"
		;;
	iodata,wn-ax1167gr2|\
	iodata,wn-ax2033gr|\
	iodata,wn-dx1167r|\
	iodata,wn-dx2033gr)
		iodata_mstc_upgrade_prepare "0xfe75"
		nand_do_upgrade "$1"
		;;
	iodata,wn-dx1200gr)
		iodata_mstc_upgrade_prepare "0x1fe75"
		nand_do_upgrade "$1"
		;;
	tplink,er605-v2)
		echo "Upgrading tplink,er605-v2"
		CI_UBIPART="firmware"
		CI_KERNPART="kernel"
		nand_do_upgrade "$1"
		;;
	ubnt,edgerouter-x|\
	ubnt,edgerouter-x-sfp)
		platform_upgrade_ubnt_erx "$1"
		;;
	zyxel,lte3301-plus|\
	zyxel,nr7101)
		fw_setenv CheckBypass 0
		fw_setenv Image1Stable 0
		[ "$(fw_printenv -n BootingFlag)" = "0" ] || fw_setenv BootingFlag 0
		CI_KERNPART="Kernel"
		nand_do_upgrade "$1"
		;;
	zyxel,wap6805)
		local kernel2_mtd="$(find_mtd_part Kernel2)"
		[ "$(hexdump -n 4 -e '"%x"' $kernel2_mtd)" = "56190527" ] &&\
		[ "$(hexdump -n 4 -s 104 -e '"%x"' $kernel2_mtd)" != "0" ] &&\
		dd bs=4 count=1 seek=26 conv=notrunc if=/dev/zero of=$kernel2_mtd 2>/dev/null &&\
		echo "Kernel2 sequence number was reset to 0"
		CI_KERNPART="Kernel"
		nand_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
