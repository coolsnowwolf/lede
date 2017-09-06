PART_NAME=firmware

# $(1): file to read magic from
# $(2): offset in bytes
get_magic_long_at() {
	dd if="$1" skip=$2 bs=1 count=4 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

platform_expected_image() {
	local machine=$(board_name)

	case "$machine" in
		"Netgear WGR614 V8")	echo "chk U12H072T00_NETGEAR"; return;;
		"Netgear WGR614 V9")	echo "chk U12H094T00_NETGEAR"; return;;
		"Netgear WGR614 V10")	echo "chk U12H139T01_NETGEAR"; return;;
		"Netgear WN2500RP V1")	echo "chk U12H197T00_NETGEAR"; return;;
		"Netgear WN2500RP V2")	echo "chk U12H294T00_NETGEAR"; return;;
		"Netgear WNDR3300")	echo "chk U12H093T00_NETGEAR"; return;;
		"Netgear WNDR3400 V1")	echo "chk U12H155T00_NETGEAR"; return;;
		"Netgear WNDR3400 V2")	echo "chk U12H187T00_NETGEAR"; return;;
		"Netgear WNDR3400 V3")	echo "chk U12H208T00_NETGEAR"; return;;
		"Netgear WNDR3400 Vcna")	echo "chk U12H155T01_NETGEAR"; return;;
		"Netgear WNDR3700 V3")	echo "chk U12H194T00_NETGEAR"; return;;
		"Netgear WNDR4000")	echo "chk U12H181T00_NETGEAR"; return;;
		"Netgear WNDR4500 V1")	echo "chk U12H189T00_NETGEAR"; return;;
		"Netgear WNDR4500 V2")	echo "chk U12H224T00_NETGEAR"; return;;
		"Netgear WNR2000 V2")	echo "chk U12H114T00_NETGEAR"; return;;
		"Netgear WNR3500L")	echo "chk U12H136T99_NETGEAR"; return;;
		"Netgear WNR3500U")	echo "chk U12H136T00_NETGEAR"; return;;
		"Netgear WNR3500 V2")	echo "chk U12H127T00_NETGEAR"; return;;
		"Netgear WNR3500 V2vc")	echo "chk U12H127T70_NETGEAR"; return;;
		"Netgear WNR834B V2")	echo "chk U12H081T00_NETGEAR"; return;;
		"Linksys E900 V1")	echo "cybertan E900"; return;;
		"Linksys E1000 V1")	echo "cybertan E100"; return;;
		"Linksys E1000 V2")	echo "cybertan E100"; return;;
		"Linksys E1000 V2.1")	echo "cybertan E100"; return;;
		"Linksys E1200 V2")	echo "cybertan E122"; return;;
		"Linksys E2000 V1")	echo "cybertan 32XN"; return;;
		"Linksys E3000 V1")	echo "cybertan 61XN"; return;;
		"Linksys E3200 V1")	echo "cybertan 3200"; return;;
		"Linksys E4200 V1")	echo "cybertan 4200"; return;;
		"Linksys WRT150N V1.1")	echo "cybertan N150"; return;;
		"Linksys WRT150N V1")	echo "cybertan N150"; return;;
		"Linksys WRT160N V1")	echo "cybertan N150"; return;;
		"Linksys WRT160N V3")	echo "cybertan N150"; return;;
		"Linksys WRT300N V1")	echo "cybertan EWCB"; return;;
		"Linksys WRT300N V1.1")	echo "cybertan EWC2"; return;;
		"Linksys WRT310N V1")	echo "cybertan 310N"; return;;
		"Linksys WRT310N V2")	echo "cybertan 310N"; return;;
		"Linksys WRT610N V1")	echo "cybertan 610N"; return;;
		"Linksys WRT610N V2")	echo "cybertan 610N"; return;;
	esac
}

brcm47xx_identify() {
	local magic

	magic=$(get_magic_long "$1")
	case "$magic" in
		"48445230")
			echo "trx"
			return
			;;
		"2a23245e")
			echo "chk"
			return
			;;
	esac

	magic=$(get_magic_long_at "$1" 14)
	[ "$magic" = "55324e44" ] && {
		echo "cybertan"
		return
	}

	echo "unknown"
}

platform_check_image() {
	[ "$#" -gt 1 ] && return 1

	local file_type=$(brcm47xx_identify "$1")
	local magic
	local error=0

	case "$file_type" in
		"chk")
			local header_len=$((0x$(get_magic_long_at "$1" 4)))
			local board_id_len=$(($header_len - 40))
			local board_id=$(dd if="$1" skip=40 bs=1 count=$board_id_len 2>/dev/null | hexdump -v -e '1/1 "%c"')
			local dev_board_id=$(platform_expected_image)
			echo "Found CHK image with device board_id $board_id"

			[ -n "$dev_board_id" -a "chk $board_id" != "$dev_board_id" ] && {
				echo "Firmware board_id doesn't match device board_id ($dev_board_id)"
				error=1
			}

			if ! otrx check "$1" -o "$header_len"; then
				echo "No valid TRX firmware in the CHK image"
				error=1
			fi
		;;
		"cybertan")
			local pattern=$(dd if="$1" bs=1 count=4 2>/dev/null | hexdump -v -e '1/1 "%c"')
			local dev_pattern=$(platform_expected_image)
			echo "Found CyberTAN image with device pattern: $pattern"

			[ -n "$dev_pattern" -a "cybertan $pattern" != "$dev_pattern" ] && {
				echo "Firmware pattern doesn't match device pattern ($dev_pattern)"
				error=1
			}

			if ! otrx check "$1" -o 32; then
				echo "No valid TRX firmware in the CyberTAN image"
				error=1
			fi
		;;
		"trx")
			if ! otrx check "$1"; then
				echo "Invalid (corrupted?) TRX firmware"
				error=1
			fi
		;;
		*)
			echo "Invalid image type. Please use only .trx files"
			error=1
		;;
	esac

	return $error
}

platform_trx_from_chk_cmd() {
	local header_len=$((0x$(get_magic_long_at "$1" 4)))

	echo -n dd bs=$header_len skip=1
}

platform_trx_from_cybertan_cmd() {
	echo -n dd bs=32 skip=1
}

platform_do_upgrade() {
	local file_type=$(brcm47xx_identify "$1")
	local trx="$1"
	local cmd=""

	case "$file_type" in
		"chk")		cmd=$(platform_trx_from_chk_cmd "$trx");;
		"cybertan")	cmd=$(platform_trx_from_cybertan_cmd "$trx");;
	esac

	default_do_upgrade "$trx" "$cmd"
}
