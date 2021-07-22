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

remove_devicename_led_sysfs() {
	local cfg="$1"; shift
	local exceptions="$@"
	local sysfs
	local name
	local new_sysfs

	config_get sysfs ${cfg} sysfs
	config_get name ${cfg} name

	# only continue if two or more colons are present
	echo "${sysfs}" | grep -q ":.*:" || return

	for exception in ${exceptions}; do
		# no change if exceptions provided as argument are found for devicename
		echo "${sysfs}" | grep -q "^${exception}:" && return
	done

	new_sysfs=$(echo ${sysfs} | sed "s/^[^:]*://")

	uci set system.${cfg}.sysfs="${new_sysfs}"

	logger -t led-migration "sysfs option of LED \"${name}\" updated to ${new_sysfs}"
}

migrate_leds() {
	config_load system
	config_foreach migrate_led_sysfs led "$@"
}

remove_devicename_leds() {
	config_load system
	config_foreach remove_devicename_led_sysfs led "$@"
}

migrations_apply() {
	local realm="$1"
	[ -n "$(uci changes ${realm})" ] && uci -q commit ${realm}
}
