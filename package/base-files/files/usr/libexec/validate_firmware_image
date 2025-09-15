#!/bin/sh

. /lib/functions.sh
. /lib/functions/system.sh
. /usr/share/libubox/jshn.sh

include /lib/upgrade

VALID=1
FORCEABLE=1
ALLOW_BACKUP=1

# Mark image as invalid but still possible to install
notify_firmware_invalid() {
	VALID=0
}

# Mark image as broken (impossible to install)
notify_firmware_broken() {
	VALID=0
	FORCEABLE=0
}

# Mark image as incompatible with preserving a backup
notify_firmware_no_backup() {
	ALLOW_BACKUP=0
}

# Add result of validation test
notify_firmware_test_result() {
	local old_ns

	json_set_namespace validate_firmware_image old_ns
	json_add_boolean "$1" "$2"
	json_set_namespace $old_ns
}

err_to_bool() {
	[ "$1" -ne 0 ] && echo 0 || echo 1
}

fwtool_check_signature "$1" >&2
FWTOOL_SIGNATURE=$?
[ "$FWTOOL_SIGNATURE" -ne 0 ] && notify_firmware_invalid

fwtool_check_image "$1" >&2
FWTOOL_DEVICE_MATCH=$?
[ "$FWTOOL_DEVICE_MATCH" -ne 0 ] && notify_firmware_invalid

json_set_namespace validate_firmware_image old_ns
json_init
	json_add_object "tests"
		json_add_boolean fwtool_signature "$(err_to_bool $FWTOOL_SIGNATURE)"
		json_add_boolean fwtool_device_match "$(err_to_bool $FWTOOL_DEVICE_MATCH)"

		# Call platform_check_image() here so it can add its test
		# results and still mark image properly.
		json_set_namespace $old_ns
		platform_check_image "$1" >&2 || notify_firmware_invalid
		json_set_namespace validate_firmware_image old_ns
	json_close_object
	json_add_boolean valid "$VALID"
	json_add_boolean forceable "$FORCEABLE"
	json_add_boolean allow_backup "$ALLOW_BACKUP"
json_dump -i
json_set_namespace $old_ns
