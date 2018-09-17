#!/bin/sh

# This must run before 10-wifi-detect


[ "${ACTION}" = "add" ] || return


. /lib/functions.sh


check_radio()
{
	local cfg="$1" to="$2"

	config_get path "$cfg" path

	[ "$path" = "$to" ] && PATH_EXISTS=true
}

do_migrate_radio()
{
	local cfg="$1" from="$2" to="$3"

	config_get path "$cfg" path

	[ "$path" = "$from" ] || return

	uci set "wireless.${cfg}.path=${to}"
	WIRELESS_CHANGED=true

	logger -t wifi-migrate "Updated path of wireless.${cfg} from '${from}' to '${to}'"
}

migrate_radio()
{
	local from="$1" to="$2"

	config_load wireless

	# Check if there is already a section with the target path: In this case, the system
	# was already upgraded to a version without this migration script before; better bail out,
	# as we can't be sure we don't break more than we fix.
	PATH_EXISTS=false
	config_foreach check_radio wifi-device "$to"
	$PATH_EXISTS && return

	config_foreach do_migrate_radio wifi-device "$from" "$to"
}


WIRELESS_CHANGED=false

case "$(board_name)" in
tplink,tl-wdr4900-v1)
	migrate_radio 'ffe09000.pcie/pci0000:00/0000:00:00.0/0000:01:00.0' 'ffe09000.pcie/pci9000:00/9000:00:00.0/9000:01:00.0'
	migrate_radio 'ffe0a000.pcie/pci0001:02/0001:02:00.0/0001:03:00.0' 'ffe0a000.pcie/pcia000:02/a000:02:00.0/a000:03:00.0'
	;;
esac

$WIRELESS_CHANGED && uci commit wireless

exit 0
