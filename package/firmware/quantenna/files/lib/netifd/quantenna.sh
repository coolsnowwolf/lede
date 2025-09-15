QUANTENNA_TARGET_IP=""
QUANTENNA_FW_VER=""
QUANTENNA_PRIMARY=""
quantenna_target_ip() {
	echo "$QUANTENNA_TARGET_IP"
}

quantenna_device() {
	[ -n "$QUANTENNA_TARGET_IP" ] && ip route get "$QUANTENNA_TARGET_IP" 2>/dev/null | sed -e 's,.* dev ,,' -e 's, .*,,' | tr -d \\n
}

quantenna_fw_ver() {
	echo "$QUANTENNA_FW_VER"
}

quantenna_api_ver() {
	echo "$QUANTENNA_FW_VER" | cut -d. -f1
}

quantenna_primaryif() {
	echo "$QUANTENNA_PRIMARY"
}

quantenna_ifprefix() {
	echo "${QUANTENNA_PRIMARY%0}"
}

# This depends on a configured qtn-utils package
[ -z "$QUANTENNA_TARGET_IP" ] && [ -r "/etc/qcsapi_target_ip.conf" ] && [ -x "$(which qcsapi_sockrpc)" ] && QUANTENNA_TARGET_IP=$(cat /etc/qcsapi_target_ip.conf)
[ -n "$QUANTENNA_TARGET_IP" ] && [ -z "$QUANTENNA_FW_VER" ] && QUANTENNA_FW_VER=$(qcsapi_sockrpc get_firmware_version)
[ -n "$QUANTENNA_TARGET_IP" ] && [ -z "$QUANTENNA_PRIMARY" ] && QUANTENNA_PRIMARY=$(qcsapi_sockrpc get_primary_interface)
