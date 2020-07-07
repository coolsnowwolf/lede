mac80211_phy_to_path() {
	local phy="$1"

	[ -x /usr/bin/readlink -a -h /sys/class/ieee80211/${phy} ] || return

	local path="$(readlink -f /sys/class/ieee80211/${phy}/device)"
	[ -n "$path" ] || return

	path="${path##/sys/devices/}"
	case "$path" in
		platform*/pci*) path="${path##platform/}";;
	esac

	local p
	local seq=""
	for p in $(ls /sys/class/ieee80211/$phy/device/ieee80211); do
		[ "$p" = "$phy" ] && {
			echo "$path${seq:++$seq}"
			break
		}

		seq=$((${seq:-0} + 1))
	done
}

mac80211_path_to_phy() {
	local path="$1"

	local p
	for p in $(ls /sys/class/ieee80211); do
		local cur="$(mac80211_phy_to_path "$p")"
		case "$cur" in
			*$path) echo "$p"; return;;
		esac
	done
}
