# SPDX-License-Identifier: GPL-2.0

get_mac_gs3101() {
	local part_name="romfile"
	local idx;

	idx=$(find_mtd_index "$part_name")
	if [ -z "$idx" ]; then
		echo "get_mac_gs3101: partition $part_name not found" >&2
		return 1
	fi

	# <Entry0 Mac="5ac876db9bf0" newHwUpBytes="0" newHwDownBytes="0"
	dd if="/dev/mtd${idx}" 2>/dev/null | \
		gunzip 2>/dev/null | \
		strings | \
		sed -n -e 's/^.*Mac="\([a-f0-9]\{12\}\)".*$/\1/p' | \
		while read mac; do
			mac=$(macaddr_canonicalize "$mac")
			[ "$mac" = "" ] && continue
			# For some reason, GS3101 stores 5a:c8:76:...
			# But the MAC address is 58:c8:76:...
			echo "$(macaddr_unsetbit "$mac" 7)"
			break
		done | \
		grep '[a-f0-9:]\{17\}' && return 0

	echo "get_mac_gs3101: could not find mac address" >&2
	return 1
}
