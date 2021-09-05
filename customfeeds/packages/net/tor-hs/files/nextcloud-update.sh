#!/bin/sh
# This is example script for tor-hs uci config
# HookScript option. Script is then called after running
# hidden service.
# It disables trusted domain check for nextcloud.

NEXTCLOUD_CLI_SCRIPT="/srv/www/nextcloud/occ"

nextcloud_cli() {
	sudo -u nobody php-cli "$NEXTCLOUD_CLI_SCRIPT" "$@"
}


nextcloud_add_domain() {
	onion="$1"
	if [ -n "$onion" ] && nextcloud_cli config:system:get trusted_domains |grep "$onion" ; then
		echo "Info: Trusted domains already disabled. Nothing to do." >&2
	else
		echo "Info: Disabling trusted domains." >&2
		nextcloud_cli config:system:set trusted_domains 1000 --value=$onion
	fi
}

print_help() {
	echo "Help"
}

# Check occ command
[ -f "$NEXTCLOUD_CLI_SCRIPT" ] || {
	echo "Error: occ command not found!" >&2
	exit 1
}

################################################################

case "$1" in
--update-onion)
	nextcloud_add_domain "$2"
;;

*)
	print_help
;;
esac
