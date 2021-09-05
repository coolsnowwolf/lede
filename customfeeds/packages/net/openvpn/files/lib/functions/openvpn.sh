#!/bin/sh

get_openvpn_option() {
	local config="$1"
	local variable="$2"
	local option="$3"

	local value="$(sed -rne 's/^[ \t]*'"$option"'[ \t]+'"'([^']+)'"'[ \t]*$/\1/p' "$config" | tail -n1)"
	[ -n "$value" ] || value="$(sed -rne 's/^[ \t]*'"$option"'[ \t]+"(([^"\\]|\\.)+)"[ \t]*$/\1/p' "$config" | tail -n1 | sed -re 's/\\(.)/\1/g')"
	[ -n "$value" ] || value="$(sed -rne 's/^[ \t]*'"$option"'[ \t]+(([^ \t\\]|\\.)+)[ \t]*$/\1/p' "$config" | tail -n1 | sed -re 's/\\(.)/\1/g')"
	[ -n "$value" ] || return 1

	export -n "$variable=$value"
	return 0
}

