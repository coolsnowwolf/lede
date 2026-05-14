#!/bin/sh

append DRIVERS "mac80211"

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d /sys/class/ieee80211/$phy ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		phy="$(iwinfo nl80211 phyname "path=$devpath")"
		[ -n "$phy" ] && return
	}

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"
	[ -n "$macaddr" ] && {
		for _phy in /sys/class/ieee80211/*; do
			[ -e "$_phy" ] || continue

			[ "$macaddr" = "$(cat ${_phy}/macaddress)" ] || continue
			phy="${_phy##*/}"
			return
		done
	}
	phy=
	return
}

find_mac80211_phy() {
	local device="$1"

	config_get phy "$device" phy
	lookup_phy
	[ -n "$phy" -a -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	config_set "$device" phy "$phy"

	config_get macaddr "$device" macaddr
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat /sys/class/ieee80211/${phy}/macaddress)"
	}

	return 0
}

check_mac80211_device() {
	local cfg="$1"
	local phy path macaddr radio

	[ -n "$check_radio" ] && {
		config_get radio "$cfg" radio
		[ "$radio" = "$check_radio" ] || return 0
	}

	config_get macaddr "$cfg" macaddr
	[ -n "$macaddr" ] && [ -n "$check_macaddr" ] && {
		macaddr="$(printf '%s' "$macaddr" | tr 'A-Z' 'a-z')"
		[ "$macaddr" = "$check_macaddr" ] && {
			found=1
			return 0
		}
	}

	config_get phy "$cfg" phy
	[ "$phy" = "$check_phy" ] && {
		found=1
		return 0
	}

	config_get path "$cfg" path
	[ -n "$path" ] && [ -n "$check_path" ] && case "$path" in
		*"$check_path")
			found=1
			return 0
		;;
	esac
}

mac80211_board_json_get() {
	local expr="$1"

	[ -s /etc/board.json ] || return 1
	command -v jsonfilter >/dev/null 2>&1 || return 1
	jsonfilter -i /etc/board.json -e "$expr" 2>/dev/null
}

mac80211_get_defaults() {
	local band="$1"

	default_country="$(mac80211_board_json_get '@.wlan.defaults.country')"
	default_ssid="$(mac80211_board_json_get "@.wlan.defaults.ssids.${band}.ssid")"
	default_encryption="$(mac80211_board_json_get "@.wlan.defaults.ssids.${band}.encryption")"
	default_key="$(mac80211_board_json_get "@.wlan.defaults.ssids.${band}.key")"
	default_mac_count="$(mac80211_board_json_get "@.wlan.defaults.ssids.${band}.mac_count")"

	[ -n "$default_ssid" ] || default_ssid="$(mac80211_board_json_get '@.wlan.defaults.ssids.all.ssid')"
	[ -n "$default_encryption" ] || default_encryption="$(mac80211_board_json_get '@.wlan.defaults.ssids.all.encryption')"
	[ -n "$default_key" ] || default_key="$(mac80211_board_json_get '@.wlan.defaults.ssids.all.key')"
}

__get_band_capabilities() {
	local phy="$1"

	iw phy "$phy" info | awk '
function band_name(id) {
	if (id == 1) return "2g"
	if (id == 2) return "5g"
	if (id == 3) return "60g"
	if (id == 4) return "6g"
	return ""
}

function flush_band() {
	if (!cur_band || !channel)
		return

	printf "band|%s|%s|%s|%d|%d|%d|%d|%s|%s\n",
		cur_band, channel, freq, ht, vht, he, eht,
		(htcap ? htcap : "0"), (hephy ? hephy : "00")
}

$1 == "Band" {
	flush_band()

	id = $2
	sub(":", "", id)
	cur_band = band_name(id + 0)
	channel = ""
	freq = ""
	ht = 0
	vht = 0
	he = 0
	eht = 0
	htcap = "0"
	hephy = "00"
	next
}

$1 == "Capabilities:" && cur_band {
	ht = 1
	htcap = $2
	next
}

$1 == "VHT" && $2 == "Capabilities" && cur_band {
	vht = 1
	next
}

$1 == "HE" && $2 == "Iftypes:" && cur_band {
	he = 1
	next
}

$1 == "EHT" && $2 == "Iftypes:" && cur_band {
	eht = 1
	next
}

$1 == "HE" && $2 == "PHY" && $3 == "Capabilities:" && cur_band {
	hephy = $0
	sub(/^.*\(0x/, "", hephy)
	hephy = substr(hephy, 1, 2)
	next
}

$1 == "*" && $3 == "MHz" && cur_band && $0 !~ /disabled/ && !channel {
	channel = $4
	gsub(/\[|\]/, "", channel)
	freq = $2
	sub(/\..*$/, "", freq)
	next
}

END {
	flush_band()
}'
}

__get_band_channels() {
	local phy="$1"

	iw phy "$phy" info | awk '
function band_name(id) {
	if (id == 1) return "2g"
	if (id == 2) return "5g"
	if (id == 3) return "60g"
	if (id == 4) return "6g"
	return ""
}

$1 == "Band" {
	id = $2
	sub(":", "", id)
	cur_band = band_name(id + 0)
	next
}

$1 == "*" && $3 == "MHz" && cur_band && $0 !~ /disabled/ {
	channel = $4
	gsub(/\[|\]/, "", channel)
	freq = $2
	sub(/\..*$/, "", freq)
	printf "freq|%s|%s|%s\n", cur_band, channel, freq
}'
}

__get_radio_ranges() {
	local phy="$1"

	iw phy "$phy" info | awk '
$1 == "wiphy" && $2 == "radio" {
	radio = $3
	sub(":", "", radio)
	next
}

$1 == "freq" && $2 == "range:" && radio != "" {
	start = $3
	end = $6
	sub(/\..*$/, "", start)
	sub(/\..*$/, "", end)
	printf "radio|%s|%s|%s\n", radio, start, end
}'
}

mac80211_get_band_width() {
	local band="$1"
	local vht="$2"
	local htcap="$3"
	local hephy="$4"
	local htcap_mask=0
	local hephy_mask=0

	[ "$band" = "2g" ] && {
		echo 20
		return
	}

	htcap_mask=$((htcap))
	hephy_mask=$((0x${hephy:-00}))

	if [ "$vht" = "1" ] || [ $((hephy_mask & 0x1c)) -gt 0 ]; then
		echo 80
	elif [ $((htcap_mask & 0x2)) -gt 0 ] || [ $((hephy_mask & 0x2)) -gt 0 ]; then
		echo 40
	else
		echo 20
	fi
}

mac80211_get_band_htmode() {
	local band="$1"
	local width="$2"
	local ht="$3"
	local vht="$4"
	local he="$5"
	local eht="$6"
	local prefix="NOHT"

	[ "$ht" = "1" ] && prefix="HT"
	[ "$vht" = "1" ] && [ "$band" != "2g" ] && prefix="VHT"
	[ "$he" = "1" ] && prefix="HE"
	[ "$eht" = "1" ] && prefix="EHT"

	[ "$prefix" = "NOHT" ] && {
		echo "$prefix"
		return
	}

	echo "${prefix}${width}"
}

mac80211_get_preferred_band() {
	local band

	for band in 6g 5g 2g 60g; do
		eval "[ -n \"\${band_${band}_channel}\" ]" && {
			echo "$band"
			return
		}
	done

	for band in $band_list; do
		echo "$band"
		return
	done
}

mac80211_get_radio_list() {
	local list="" kind radio start end

	while IFS='|' read -r kind radio start end; do
		[ "$kind" = "radio" ] || continue
		case " $list " in
			*" $radio "*) ;;
			*) list="$list $radio" ;;
		esac
	done <<-EOF
	$radio_ranges
	EOF

	echo "${list# }"
}

mac80211_find_radio_band_channel() {
	local radio="$1"
	local band="$2"
	local kind cur_radio start end cur_band channel freq

	while IFS='|' read -r kind cur_radio start end; do
		[ "$kind" = "radio" ] || continue
		[ "$cur_radio" = "$radio" ] || continue

		while IFS='|' read -r kind cur_band channel freq; do
			[ "$kind" = "freq" ] || continue
			[ "$cur_band" = "$band" ] || continue
			[ "$freq" -ge "$start" ] && [ "$freq" -le "$end" ] && {
				echo "$channel"
				return
			}
		done <<-EOF_INNER
		$band_channels
		EOF_INNER
	done <<-EOF
	$radio_ranges
	EOF
}

mac80211_emit_detect() {
	local dev="$1"
	local path="$2"
	local band="$3"
	local channel="$4"
	local htmode="$5"
	local radio="$6"
	local country encryption ssid key num_global_macaddr
	local section="radio${devidx}"
	local iface="default_radio${devidx}"
	local dev_id

	mac80211_get_defaults "$band"

	country="$default_country"
	encryption="$default_encryption"
	ssid="$default_ssid"
	key="$default_key"
	num_global_macaddr="$default_mac_count"

	[ -n "$ssid" ] || ssid="LEDE"
	[ -n "$country" ] || country="US"

	case "$band" in
		6g)
			[ -n "$default_country" ] || country="00"
			[ -n "$encryption" ] || encryption="owe"
		;;
		*)
			[ -n "$encryption" ] || encryption="none"
		;;
	esac

	case "$dev" in
		phy[0-9]*)
			if [ -n "$path" ]; then
				dev_id="set wireless.${section}.path='${path}'"
			else
				dev_id="set wireless.${section}.phy='${dev}'"
			fi
		;;
		*)
			dev_id="set wireless.${section}.phy='${dev}'"
		;;
	esac

	[ -n "$radio" ] && dev_id="${dev_id}
			set wireless.${section}.radio='${radio}'"

	uci -q batch <<-EOF
		set wireless.${section}=wifi-device
		set wireless.${section}.type='mac80211'
		${dev_id}
		set wireless.${section}.band='${band}'
		set wireless.${section}.channel='${channel}'
		set wireless.${section}.htmode='${htmode}'
		set wireless.${section}.country='${country}'
		set wireless.${section}.num_global_macaddr='${num_global_macaddr}'
		set wireless.${section}.disabled='0'

		set wireless.${iface}=wifi-iface
		set wireless.${iface}.device='${section}'
		set wireless.${iface}.network='lan'
		set wireless.${iface}.mode='ap'
		set wireless.${iface}.ssid='${ssid}'
		set wireless.${iface}.encryption='${encryption}'
		set wireless.${iface}.key='${key}'
		set wireless.${iface}.disabled='0'
	EOF
	uci -q commit wireless
}

detect_mac80211() {
	devidx=0
	config_load wireless
	while :; do
		config_get type "radio$devidx" type
		[ -n "$type" ] || break
		devidx=$(($devidx + 1))
	done

	for _dev in /sys/class/ieee80211/*; do
		[ -e "$_dev" ] || continue

		dev="${_dev##*/}"
		path="$(iwinfo nl80211 path "$dev")"
		check_macaddr="$(cat /sys/class/ieee80211/${dev}/macaddress)"

		band_list=""
		band_caps="$(__get_band_capabilities "$dev")"
		band_channels="$(__get_band_channels "$dev")"
		radio_ranges="$(__get_radio_ranges "$dev")"

		while IFS='|' read -r kind band chan freq ht vht he eht htcap hephy; do
			local width mode

			[ "$kind" = "band" ] || continue
			width="$(mac80211_get_band_width "$band" "$vht" "$htcap" "$hephy")"
			mode="$(mac80211_get_band_htmode "$band" "$width" "$ht" "$vht" "$he" "$eht")"

			append band_list "$band"
			eval "band_${band}_channel='$chan'"
			eval "band_${band}_htmode='$mode'"
		done <<-EOF
		$band_caps
		EOF

		[ -n "$band_list" ] || continue

		radios="$(mac80211_get_radio_list)"
		if [ -n "$radios" ]; then
			for radio in $radios; do
				selected_band=""
				selected_channel=""
				selected_htmode=""

				for candidate_band in 6g 5g 2g 60g; do
					selected_channel="$(mac80211_find_radio_band_channel "$radio" "$candidate_band")"
					[ -n "$selected_channel" ] || continue

					selected_band="$candidate_band"
					eval "selected_htmode=\"\${band_${candidate_band}_htmode}\""
					break
				done

				[ -n "$selected_band" ] || continue

				found=0
				check_phy="$dev"
				check_path="$path"
				check_radio="$radio"
				config_foreach check_mac80211_device wifi-device
				[ "$found" -gt 0 ] && continue

				mac80211_emit_detect "$dev" "$path" "$selected_band" "$selected_channel" "$selected_htmode" "$radio"
				devidx=$(($devidx + 1))
			done
			continue
		fi

		found=0
		check_phy="$dev"
		check_path="$path"
		check_radio=""
		config_foreach check_mac80211_device wifi-device
		[ "$found" -gt 0 ] && continue

		selected_band="$(mac80211_get_preferred_band)"
		[ -n "$selected_band" ] || continue

		eval "selected_channel=\"\${band_${selected_band}_channel}\""
		eval "selected_htmode=\"\${band_${selected_band}_htmode}\""

		mac80211_emit_detect "$dev" "$path" "$selected_band" "$selected_channel" "$selected_htmode"

		devidx=$(($devidx + 1))
	done
}
