PART_NAME=firmware

LXL_FLAGS_VENDOR_LUXUL=0x00000001

# $(1): file to read magic from
# $(2): offset in bytes
get_magic_long_at() {
	dd if="$1" skip=$2 bs=1 count=4 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

# $(1): file to read LE long number from
# $(2): offset in bytes
get_le_long_at() {
	echo $((0x$(dd if="$1" skip=$2 bs=1 count=4 2>/dev/null | hexdump -v -e '1/4 "%02x"')))
}

platform_expected_image() {
	local model="$(cat /tmp/sysinfo/model)"

	case "$model" in
		"Netgear R6200 V1")	echo "chk U12H192T00_NETGEAR"; return;;
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
		"Luxul XAP-310 V1")	echo "lxl XAP-310"; return;;
		"Luxul XAP-1210 V1")	echo "lxl XAP-1210"; return;;
		"Luxul XAP-1230 V1")	echo "lxl XAP-1230"; return;;
		"Luxul XAP-1240 V1")	echo "lxl XAP-1240"; return;;
		"Luxul XAP-1500 V1")	echo "lxl XAP-1500"; return;;
		"Luxul ABR-4400 V1")	echo "lxl ABR-4400"; return;;
		"Luxul XBR-4400 V1")	echo "lxl XBR-4400"; return;;
		"Luxul XVW-P30 V1")	echo "lxl XVW-P30"; return;;
		"Luxul XWR-600 V1")	echo "lxl XWR-600"; return;;
		"Luxul XWR-1750 V1")	echo "lxl XWR-1750"; return;;
	esac
}

bcm47xx_identify() {
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
		"4c584c23")
			echo "lxl"
			return
			;;
	esac

	magic=$(get_magic_long_at "$1" 14)
	[ "$magic" = "55324e44" ] && {
		echo "cybertan"
		return
	}

	magic=$(get_magic_long_at "$1" 60)
	[ "$magic" = "4c584c23" ] && {
		echo "lxlold"
		return
	}

	echo "unknown"
}

platform_check_image() {
	[ "$#" -gt 1 ] && return 1

	local file_type=$(bcm47xx_identify "$1")
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
				notify_firmware_test_result "trx_valid" 0
				error=1
			else
				notify_firmware_test_result "trx_valid" 1
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
				notify_firmware_test_result "trx_valid" 0
				error=1
			else
				notify_firmware_test_result "trx_valid" 1
			fi
		;;
		"lxl")
			local hdr_len=$(get_le_long_at "$1" 8)
			local flags=$(get_le_long_at "$1" 12)
			local board=$(dd if="$1" skip=16 bs=1 count=16 2>/dev/null | hexdump -v -e '1/1 "%c"')
			local dev_board=$(platform_expected_image)
			echo "Found Luxul image for board $board"

			[ -n "$dev_board" -a "lxl $board" != "$dev_board" ] && {
				echo "Firmware ($board) doesn't match device ($dev_board)"
				error=1
			}

			[ $((flags & LXL_FLAGS_VENDOR_LUXUL)) -gt 0 ] && notify_firmware_no_backup

			if ! otrx check "$1" -o "$hdr_len"; then
				echo "No valid TRX firmware in the Luxul image"
				notify_firmware_test_result "trx_valid" 0
				error=1
			else
				notify_firmware_test_result "trx_valid" 1
			fi
		;;
		"lxlold")
			local board_id=$(dd if="$1" skip=48 bs=1 count=12 2>/dev/null | hexdump -v -e '1/1 "%c"')
			local dev_board_id=$(platform_expected_image)
			echo "Found Luxul image with device board_id $board_id"

			[ -n "$dev_board_id" -a "lxl $board_id" != "$dev_board_id" ] && {
				echo "Firmware board_id doesn't match device board_id ($dev_board_id)"
				error=1
			}

			notify_firmware_no_backup

			if ! otrx check "$1" -o 64; then
				echo "No valid TRX firmware in the Luxul image"
				notify_firmware_test_result "trx_valid" 0
				error=1
			else
				notify_firmware_test_result "trx_valid" 1
			fi
		;;
		"trx")
			if ! otrx check "$1"; then
				echo "Invalid (corrupted?) TRX firmware"
				notify_firmware_test_result "trx_valid" 0
				error=1
			else
				notify_firmware_test_result "trx_valid" 1
			fi
		;;
		*)
			echo "Invalid image type. Please use firmware specific for this device."
			notify_firmware_broken
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

platform_trx_from_lxl_cmd() {
	local hdr_len=$(get_le_long_at "$1" 8)

	echo -n dd skip=$hdr_len iflag=skip_bytes
}

platform_trx_from_lxlold_cmd() {
	echo -n dd bs=64 skip=1
}

platform_do_upgrade() {
	local file_type=$(bcm47xx_identify "$1")
	local trx="$1"
	local cmd=""

	case "$file_type" in
		"chk")		cmd=$(platform_trx_from_chk_cmd "$trx");;
		"cybertan")	cmd=$(platform_trx_from_cybertan_cmd "$trx");;
		"lxl")		cmd=$(platform_trx_from_lxl_cmd "$trx");;
		"lxlold")	cmd=$(platform_trx_from_lxlold_cmd "$trx");;
	esac

	default_do_upgrade "$trx" "$cmd"
}
