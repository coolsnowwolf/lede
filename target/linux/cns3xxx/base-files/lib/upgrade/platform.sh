PART_NAME=firmware

CI_BLKSZ=65536

platform_check_image() {
	local magic="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	[ "$magic" != "27051956" ] && {
		echo "Invalid image type."
		return 1
	}
	return 0
}

platform_do_upgrade() {
	default_do_upgrade "$ARGV"
}

disable_watchdog() {
	v "killing watchdog"
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}

# CONFIG_WATCHDOG_NOWAYOUT=y - can't kill watchdog unless kernel cmdline has a mpcore_wdt.nowayout=0
#append sysupgrade_pre_upgrade disable_watchdog
