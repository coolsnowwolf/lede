#!/bin/sh

. /lib/functions.sh

get_onion_hostname() {
	local name="$1"
	config_get hs_dir common HSDir
	if [ -f "$hs_dir/$name/hostname" ]; then
	       cat "$hs_dir/$name/hostname"
	fi
}

get_port_list() {
	local config="$1"
	config_get ports "$config" PublicLocalPort
	tmp="$(echo $ports |sed "s| |','|g")"
	echo -ne "['$tmp']"
}

parse_hs_conf() {
	local name description public_port local_port enable_bool public_local_port ipv4
	local config="$1"
	local custom="$2"

	config_get name "$config" Name
	config_get description "$config" Description

	config_get_bool enable_hs "$config" Enabled 0
	config_get ipv4 "$config" IPv4

	hostname="$(get_onion_hostname $name)"
	port_list="$(get_port_list $config)"
	echo "{"
	echo \"name\":\"$name\",
	echo \"description\":\"$description\",
	echo \"enabled\":\"$enable_hs\",
	echo \"ipv4\":\"$ipv4\",
	echo \"hostname\":\"$hostname\",
	echo \"ports\":$port_list
	echo "},"
}

get_tor_hs_list() {
	config_load tor-hs
	echo "{"
	echo '"hs-list":['
	config_foreach parse_hs_conf hidden-service
	echo "]"
	echo "}"
}



case "$1" in
	list)
		echo '{ "list-hs": { } }'
	;;
	call)
		case "$2" in
			list-hs)
				# return json object
				get_tor_hs_list
			;;
		esac
	;;
esac



