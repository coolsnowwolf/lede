#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	adtran,bsap1800-v2|\
	adtran,bsap1840)
		fconfig -s -w -d $(find_mtd_part "RedBoot config") -n boot_cntb -x 0
		;;
	qihoo,c301)
		local n=$(fw_printenv activeregion | cut -d = -f 2)
		fw_setenv "image${n}trynum" 0
		;;
	esac
}
