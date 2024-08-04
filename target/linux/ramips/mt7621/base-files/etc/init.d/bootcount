#!/bin/sh /etc/rc.common

START=99

boot() {
	case $(board_name) in
	alfa-network,quad-e4g)
		[ -n "$(fw_printenv bootcount bootchanged 2>/dev/null)" ] &&\
			echo -e "bootcount\nbootchanged\n" | /usr/sbin/fw_setenv -s -
		;;
	beeline,smartbox-turbo-plus)
		[[ $(hexdump -n 1 -e '/1 "%1d"' -s $((0x20001)) /dev/mtd3) == \
			$((0xFF)) ]] || printf '\xff' | dd of=/dev/mtdblock3 count=1 \
			bs=1 seek=$((0x20001))
		;;
	jdcloud,re-cp-02)
		echo -e "bootcount 0\nbootlimit 5\nupgrade_available 1"  | /usr/sbin/fw_setenv -s -
		;;
	linksys,e5600|\
	linksys,ea7300-v1|\
	linksys,ea7300-v2|\
	linksys,ea7500-v2|\
	linksys,ea8100-v1|\
	linksys,ea8100-v2)
		mtd resetbc s_env || true
		;;
	samknows,whitebox-v8)
		fw_setenv bootcount 0
		;;
	zyxel,nr7101)
		[ $(printf %d $(fw_printenv -n DebugFlag)) -gt 0 ] || fw_setenv DebugFlag 0x1
		[ $(printf %d $(fw_printenv -n Image1Stable)) -gt 0 ] || fw_setenv Image1Stable 1
		[ $(printf %d $(fw_printenv -n Image1Try)) -gt 0 ] && fw_setenv Image1Try 0
		;;
	esac
}
