#!/bin/sh

OPKG_KEYS="${OPKG_KEYS:-/etc/opkg/keys}"

usage() {
	cat <<EOF
Usage: $0 <command> <arguments...>
Commands:
  add <file>:			Add keyfile <file> to opkg trusted keys
  remove <file>:		Remove keyfile matching <file> from opkg trusted keys
  verify <sigfile> <list>:	Check list file <list> against signature file <sigfile>

EOF
	exit 1
}

opkg_key_verify() {
	local sigfile="$1"
	local msgfile="$2"

	(
		zcat "$msgfile" 2>/dev/null ||
		cat "$msgfile" 2>/dev/null
	) | usign -V -P "$OPKG_KEYS" -q -x "$sigfile" -m -
}

opkg_key_add() {
	local key="$1"
	[ -n "$key" ] || usage
	[ -f "$key" ] || echo "Cannot open file $1"
	local fingerprint="$(usign -F -p "$key")"
	mkdir -p "$OPKG_KEYS"
	cp "$key" "$OPKG_KEYS/$fingerprint"
}

opkg_key_remove() {
	local key="$1"
	[ -n "$key" ] || usage
	[ -f "$key" ] || echo "Cannot open file $1"
	local fingerprint="$(usign -F -p "$key")"
	rm -f "$OPKG_KEYS/$fingerprint"
}

case "$1" in
	add)
		shift
		opkg_key_add "$@"
		;;
	remove)
		shift
		opkg_key_remove "$@"
		;;
	verify)
		shift
		opkg_key_verify "$@"
		;;
	*) usage ;;
esac
