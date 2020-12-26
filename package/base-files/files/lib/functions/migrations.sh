#!/bin/sh

. /lib/functions.sh

migrate_led_sysfs() {
	local cfg="$1"; shift
	local tuples="$@"
	local sysfs
	local name

	config_get sysfs ${cfg} sysfs
	config_get name ${cfg} name

	[ -z "${sysfs}" ] && return

	for tuple in ${tuples}; do
		local old=${tuple%=*}
		local new=${tuple#*=}
		local new_sysfs

		new_sysfs=$(echo ${sysfs} | sed "s/${old}/${new}/")

		[ "${new_sysfs}" = "${sysfs}" ] && continue

		uci set system.${cfg}.sysfs="${new_sysfs}"

		logger -t led-migration "sysfs option of LED \"${name}\" updated to ${new_sysfs}"
	done;
}

migrate_leds() {
	config_load system
	config_foreach migrate_led_sysfs led "$@"
}

migrations_apply() {
	local realm="$1"
	[ -n "$(uci changes ${realm})" ] && uci -q commit ${realm}
}
