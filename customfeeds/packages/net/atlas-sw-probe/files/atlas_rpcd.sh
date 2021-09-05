#!/bin/sh

. /lib/functions.sh

SCRIPTS_DIR="/usr/libexec/atlas-probe-scripts"
TMP_BASE_DIR="/tmp/ripe_atlas_probe"
PUB_KEY_FILE="$SCRIPTS_DIR/etc/probe_key.pub"
PRIV_KEY_FILE="$SCRIPTS_DIR/etc/probe_key"
PROBE_ID_FILE="$TMP_BASE_DIR/status/reg_init_reply.txt"

get_atlas_public_key() {
	local pub_key

	if [ -f "$PUB_KEY_FILE" ]; then
		pub_key=$(cat "$PUB_KEY_FILE")
	fi

	echo "{"
	echo \"pub-key\":\"$pub_key\"
	echo "}"
}

get_atlas_probeid() {
	local probe_id

	if /etc/init.d/atlas probeid 2>/dev/null; then
		probe_id="$(awk '/PROBE_ID/ {print $2}' "$PROBE_ID_FILE")"
	fi

	echo "{"
	echo \"probe-id\":\"$probe_id\"
	echo "}"
}

get_reg_info() {
	local pub_ip
	local asn
	local asn_org

	if [ -z "$pub_ip" ]; then
		pub_ip="$(dig -4 TXT +short o-o.myaddr.l.google.com @ns1.google.com|tr -d '"')"
	fi

	echo "{"
	echo \"public-ipv4\":\"$pub_ip\"
	echo "}"
}

get_status() {
	local status

	status="$(/etc/init.d/atlas status)"
	echo "{"
	echo \"status\":\"$status\"
	echo "}"
}

case "$1" in
	list)
		echo '{'
		echo ' "pub-key": {},'
		echo ' "probe-id": {},'
		echo ' "reg-info": {}'
		echo ' "status": {}'
		echo '}'
	;;
	call)
		case "$2" in
			pub-key)
				get_atlas_public_key
			;;
			probe-id)
				get_atlas_probeid
			;;
			reg-info)
				get_reg_info
			;;
			get-status)
				get_status
			;;
		esac
	;;
esac
