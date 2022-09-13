RAMFS_COPY_BIN='osafeloader oseama otrx truncate'

PART_NAME=firmware

BCM53XX_FW_FORMAT=
BCM53XX_FW_BOARD_ID=
BCM53XX_FW_INT_IMG_FORMAT=
BCM53XX_FW_INT_IMG_TRX_OFFSET=
BCM53XX_FW_INT_IMG_EXTRACT_CMD=

LXL_FLAGS_VENDOR_LUXUL=0x00000001

# $(1): file to read magic from
# $(2): offset in bytes
get_magic_long_at() {
	dd if="$1" skip=$2 bs=1 count=4 2>/dev/null | hexdump -v -e '1/1 "%02x"'
}

# $(1): file to read LE long number from
# $(2): offset in bytes
get_le_long_at() {
	echo $((0x$(dd if="$1" skip=$2 bs=1 count=4 2>/dev/null | hexdump -v -e '1/4 "%02x"')))
}

platform_flash_type() {
	# On NAND devices "rootfs" is UBI volume, so won't be find in /proc/mtd
	grep -q "\"rootfs\"" /proc/mtd && {
		echo "serial"
		return
	}

	echo "nand"
}

platform_expected_image() {
	local machine=$(board_name)

	case "$machine" in
		"dlink,dir-885l")	echo "seamaseal wrgac42_dlink.2015_dir885l"; return;;
		"luxul,abr-4500-v1")	echo "lxl ABR-4500"; return;;
		"luxul,xap-810-v1")	echo "lxl XAP-810"; return;;
		"luxul,xap-1410v1")	echo "lxl XAP-1410"; return;;
		"luxul,xap-1440-v1")	echo "lxl XAP-1440"; return;;
		"luxul,xap-1510v1")	echo "lxl XAP-1510"; return;;
		"luxul,xap-1610-v1")	echo "lxl XAP-1610"; return;;
		"luxul,xbr-4500-v1")	echo "lxl XBR-4500"; return;;
		"luxul,xwc-1000")	echo "lxl XWC-1000"; return;;
		"luxul,xwc-2000-v1")	echo "lxl XWC-2000"; return;;
		"luxul,xwr-1200v1")	echo "lxl XWR-1200"; return;;
		"luxul,xwr-3100v1")	echo "lxl XWR-3100"; return;;
		"luxul,xwr-3150-v1")	echo "lxl XWR-3150"; return;;
		"netgear,r6250v1")	echo "chk U12H245T00_NETGEAR"; return;;
		"netgear,r6300v2")	echo "chk U12H240T00_NETGEAR"; return;;
		"netgear,r7000")	echo "chk U12H270T00_NETGEAR"; return;;
		"netgear,r7900")	echo "chk U12H315T30_NETGEAR"; return;;
		"netgear,r8000")	echo "chk U12H315T00_NETGEAR"; return;;
		"netgear,r8500")	echo "chk U12H334T00_NETGEAR"; return;;
		"tplink,archer-c9-v1")	echo "safeloader"; return;;
	esac
}

platform_identify() {
	local magic

	magic=$(get_magic_long "$1")
	case "$magic" in
		"48445230")
			BCM53XX_FW_FORMAT="trx"
			return
			;;
		"2a23245e")
			local header_len=$((0x$(get_magic_long_at "$1" 4)))
			local board_id_len=$(($header_len - 40))

			BCM53XX_FW_FORMAT="chk"
			BCM53XX_FW_BOARD_ID=$(dd if="$1" skip=40 bs=1 count=$board_id_len 2>/dev/null | hexdump -v -e '1/1 "%c"')
			BCM53XX_FW_INT_IMG_FORMAT="trx"
			BCM53XX_FW_INT_IMG_TRX_OFFSET="$header_len"
			BCM53XX_FW_INT_IMG_EXTRACT_CMD="dd skip=$header_len iflag=skip_bytes"
			return
			;;
		"4c584c23")
			local hdr_len=$(get_le_long_at "$1" 8)
			local flags=$(get_le_long_at "$1" 12)

			[ $((flags & LXL_FLAGS_VENDOR_LUXUL)) -gt 0 ] && notify_firmware_no_backup

			BCM53XX_FW_FORMAT="lxl"
			BCM53XX_FW_BOARD_ID=$(dd if="$1" skip=16 bs=1 count=16 2>/dev/null | hexdump -v -e '1/1 "%c"')
			BCM53XX_FW_INT_IMG_FORMAT="trx"
			BCM53XX_FW_INT_IMG_TRX_OFFSET="$hdr_len"
			BCM53XX_FW_INT_IMG_EXTRACT_CMD="dd skip=$hdr_len iflag=skip_bytes"

			return
			;;
		"5ea3a417")
			BCM53XX_FW_FORMAT="seamaseal"
			BCM53XX_FW_BOARD_ID=$(oseama info "$1" | grep "Meta entry:.*signature=" | sed "s/.*=//")
			BCM53XX_FW_INT_IMG_EXTRACT_CMD="oseama extract - -e 0"
			return
			;;
	esac

	magic=$(get_magic_long_at "$1" 14)
	[ "$magic" = "55324e44" ] && {
		BCM53XX_FW_FORMAT="cybertan"
		BCM53XX_FW_BOARD_ID=$(dd if="$1" bs=1 count=4 2>/dev/null | hexdump -v -e '1/1 "%c"')
		BCM53XX_FW_INT_IMG_FORMAT="trx"
		BCM53XX_FW_INT_IMG_TRX_OFFSET="32"
		BCM53XX_FW_INT_IMG_EXTRACT_CMD="dd skip=32 iflag=skip_bytes"
		return
	}

	magic=$(get_magic_long_at "$1" 60)
	[ "$magic" = "4c584c23" ] && {
		notify_firmware_no_backup

		BCM53XX_FW_FORMAT="lxlold"
		BCM53XX_FW_BOARD_ID=$(dd if="$1" skip=48 bs=1 count=12 2>/dev/null | hexdump -v -e '1/1 "%c"')
		BCM53XX_FW_INT_IMG_FORMAT="trx"
		BCM53XX_FW_INT_IMG_TRX_OFFSET="64"
		BCM53XX_FW_INT_IMG_EXTRACT_CMD="dd skip=64 iflag=skip_bytes"
		return
	}

	if osafeloader info "$1" > /dev/null 2>&1; then
		BCM53XX_FW_FORMAT="safeloader"
		return
	fi
}

platform_other_check_image() {
	[ "$#" -gt 1 ] && return 1

	local error=0

	platform_identify "$1"
	[ -z "$BCM53XX_FW_FORMAT" ] && {
		echo "Invalid image type. Please use firmware specific for this device."
		notify_firmware_broken
		return 1
	}
	echo "Found $BCM53XX_FW_FORMAT firmware for device $BCM53XX_FW_BOARD_ID"

	local expected_image="$(platform_expected_image)"
	local tmp_format=$BCM53XX_FW_FORMAT
	[ "$tmp_format" = "lxlold" ] && tmp_format="lxl"
	[ -n "$expected_image" -a -n "$BCM53XX_FW_BOARD_ID" -a "$expected_image" != "$tmp_format $BCM53XX_FW_BOARD_ID" ] && {
		echo "Firmware doesn't match device ($expected_image)"
		error=1
	}

	case "$BCM53XX_FW_FORMAT" in
		"seamaseal")
			$(oseama info "$1" -e 0 | grep -q "Meta entry:.*type=firmware") || {
				echo "Seama seal doesn't contain firmware entity"
				error=1
			}
			;;
		"trx")
			if ! otrx check "$1"; then
				echo "Failed to find a valid TRX in firmware"
				notify_firmware_test_result "trx_valid" 0
				error=1
			else
				notify_firmware_test_result "trx_valid" 1
			fi

			[ "$expected_image" == "safeloader" ] && {
				echo "This device expects SafeLoader format and may not work with TRX"
				error=1
			}
			;;
		*)
			case "$BCM53XX_FW_INT_IMG_FORMAT" in
				"trx")
					# Make sure that both ways of extracting TRX work.
					# platform_do_upgrade() may use any of them.
					if ! otrx check "$1" -o "$BCM53XX_FW_INT_IMG_TRX_OFFSET" || \
					   ! $BCM53XX_FW_INT_IMG_EXTRACT_CMD < $1 | otrx check -; then
						echo "Invalid (corrupted?) TRX firmware"
						notify_firmware_test_result "trx_valid" 0
						error=1
					else
						notify_firmware_test_result "trx_valid" 1
					fi
					;;
			esac
			;;
	esac

	return $error
}

platform_check_image() {
	local board

	board="$(board_name)"
	case "$board" in
	# Ideally, REQUIRE_IMAGE_METADATA=1 would suffice
	# but this would require converting all other
	# devices too.
	meraki,mr26 | \
	meraki,mr32)
		nand_do_platform_check "${board//,/_}" "$1"
		return $?
		;;
	*)
		platform_other_check_image "$1"
		return $?
		;;
	esac

	return 1
}


# $(1): TRX image or firmware containing TRX
# $(2): offset of TRX in firmware (optional)
platform_do_upgrade_nand_trx() {
	local dir="/tmp/sysupgrade-bcm53xx"
	local trx="$1"
	local offset="$2"

	# Extract partitions from trx
	rm -fR $dir
	mkdir -p $dir
	otrx extract "$trx" \
		${offset:+-o $offset} \
		-1 $dir/kernel \
		-2 $dir/root
	[ $? -ne 0 ] && {
		echo "Failed to extract TRX partitions."
		return
	}

	# Firmwares without UBI image should be flashed "normally"
	local root_type=$(identify $dir/root)
	[ "$root_type" != "ubi" ] && {
		echo "Provided firmware doesn't use UBI for rootfs."
		return
	}

	# Prepare TRX file with just a kernel that will replace current one
	local linux_length=$(grep "\"linux\"" /proc/mtd | sed "s/mtd[0-9]*:[ \t]*\([^ \t]*\).*/\1/")
	[ -z "$linux_length" ] && {
		echo "Unable to find \"linux\" partition size"
		exit 1
	}
	linux_length=$((0x$linux_length))
	local kernel_length=$(wc -c $dir/kernel | cut -d ' ' -f 1)
	[ $kernel_length -gt $linux_length ] && {
		echo "New kernel doesn't fit \"linux\" partition."
		return
	}
	rm -f /tmp/null.bin
	rm -f /tmp/kernel.trx
	touch /tmp/null.bin
	otrx create /tmp/kernel.trx \
		-f $dir/kernel -b $(($linux_length + 28)) \
		-f /tmp/null.bin
	[ $? -ne 0 ] && {
		echo "Failed to create simple TRX with new kernel."
		return
	}

	# Prepare UBI image (drop unwanted extra blocks)
	local ubi_length=0
	while [ "$(dd if=$dir/root skip=$ubi_length bs=1 count=4 2>/dev/null)" = "UBI#" ]; do
		ubi_length=$(($ubi_length + 131072))
	done
	truncate -s $ubi_length $dir/root
	[ $? -ne 0 ] && {
		echo "Failed to prepare new UBI image."
		return
	}

	# Flash
	mtd write /tmp/kernel.trx firmware || exit 1
	nand_do_upgrade $dir/root
}

platform_do_upgrade_nand_seamaseal() {
	local dir="/tmp/sysupgrade-bcm53xx"
	local seamaseal="$1"
	local tmp

	# Extract Seama entity from Seama seal
	rm -fR $dir
	mkdir -p $dir
	oseama extract "$seamaseal" \
		-e 0 \
		-o $dir/seama.entity
	[ $? -ne 0 ] && {
		echo "Failed to extract Seama entity."
		return
	}
	local entity_size=$(wc -c $dir/seama.entity | cut -d ' ' -f 1)

	local ubi_offset=0
	tmp=0
	while [ 1 ]; do
		[ $tmp -ge $entity_size ] && break
		[ "$(dd if=$dir/seama.entity skip=$tmp bs=1 count=4 2>/dev/null)" = "UBI#" ] && {
			ubi_offset=$tmp
			break
		}
		tmp=$(($tmp + 131072))
	done
	[ $ubi_offset -eq 0 ] && {
		echo "Failed to find UBI in Seama entity."
		return
	}

	local ubi_length=0
	while [ "$(dd if=$dir/seama.entity skip=$(($ubi_offset + $ubi_length)) bs=1 count=4 2>/dev/null)" = "UBI#" ]; do
		ubi_length=$(($ubi_length + 131072))
	done

	dd if=$dir/seama.entity of=$dir/kernel.seama bs=131072 count=$(($ubi_offset / 131072)) 2>/dev/null
	dd if=$dir/seama.entity of=$dir/root.ubi bs=131072 skip=$(($ubi_offset / 131072)) count=$(($ubi_length / 131072)) 2>/dev/null

	# Flash
	local kernel_size=$(sed -n 's/mtd[0-9]*: \([0-9a-f]*\).*"\(kernel\|linux\)".*/\1/p' /proc/mtd)
	mtd write $dir/kernel.seama firmware || exit 1
	mtd ${kernel_size:+-c 0x$kernel_size} fixseama firmware
	nand_do_upgrade $dir/root.ubi
}

platform_img_from_safeloader() {
	local dir="/tmp/sysupgrade-bcm53xx"

	# Extract partitions from SafeLoader
	rm -fR $dir
	mkdir -p $dir
	osafeloader extract "$1" \
		-p "os-image" \
		-o $dir/os-image
	osafeloader extract "$1" \
		-p "file-system" \
		-o $dir/file-system

	mtd write $dir/file-system rootfs

	echo -n $dir/os-image
}

platform_other_do_upgrade() {
	platform_identify "$1"

	[ "$(platform_flash_type)" == "nand" ] && {
		# Try NAND-aware upgrade
		case "$BCM53XX_FW_FORMAT" in
			"seamaseal")
				platform_do_upgrade_nand_seamaseal "$1"
				;;
			"trx")
				platform_do_upgrade_nand_trx "$1"
				;;
			*)
				case "$BCM53XX_FW_INT_IMG_FORMAT" in
					"trx")
						platform_do_upgrade_nand_trx "$1" "$BCM53XX_FW_INT_IMG_TRX_OFFSET"
						;;
					*)
						echo "NAND aware sysupgrade is unsupported for $BCM53XX_FW_FORMAT format"
						;;
				esac
				;;
		esac

		# Above calls exit on success.
		# If we got here something went wrong.
		echo "Writing whole image to NAND flash. All erase counters will be lost."
	}

	case "$BCM53XX_FW_FORMAT" in
		"safeloader")
			PART_NAME=os-image
			img=$(platform_img_from_safeloader "$1")
			default_do_upgrade "$img"
			;;
		"seamaseal")
			default_do_upgrade "$1" "$BCM53XX_FW_INT_IMG_EXTRACT_CMD"
			;;
		"trx")
			default_do_upgrade "$1"
			;;
		*)
			case "$BCM53XX_FW_INT_IMG_FORMAT" in
				"trx")
					default_do_upgrade "$1" "$BCM53XX_FW_INT_IMG_EXTRACT_CMD"
					;;
			esac
			;;
	esac
}

platform_do_upgrade() {
	case "$(board_name)" in
	meraki,mr26 | \
	meraki,mr32)
		CI_KERNPART="part.safe"
		nand_do_upgrade "$1"
		;;
	*)
		platform_other_do_upgrade "$1"
		;;
	esac
}
