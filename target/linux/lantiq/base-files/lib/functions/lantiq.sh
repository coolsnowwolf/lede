#!/bin/sh

lantiq_get_dt_led() {
	local label
	local ledpath
	local basepath="/proc/device-tree"
	local nodepath="$basepath/aliases/led-$1"

	[ -f "$nodepath" ] && ledpath=$(cat "$nodepath")
	[ -n "$ledpath" ] && label=$(cat "$basepath$ledpath/label")

	echo "$label"
}

lantiq_is_vdsl_system() {
	grep -qE "system type.*: (VR9|xRX200)" /proc/cpuinfo
}
