RAMFS_COPY_BIN='osafeloader oseama otrx truncate'

PART_NAME=firmware

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
		"dlink,dir-885l")	echo "seama wrgac42_dlink.2015_dir885l"; return;;
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
		"5ea3a417")
			echo "seama"
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

	if osafeloader info "$1" > /dev/null 2>&1; then
		echo "safeloader"
		return
	fi

	echo "unknown"
}

platform_check_image() {
	[ "$#" -gt 1 ] && return 1

	local file_type=$(platform_identify "$1")
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
			echo "Found LXL image for board $board"

			[ -n "$dev_board" -a "lxl $board" != "$dev_board" ] && {
				echo "Firmware ($board) doesn't match device ($dev_board)"
				error=1
			}

			[ $((flags & LXL_FLAGS_VENDOR_LUXUL)) -gt 0 ] && notify_firmware_no_backup

			if ! otrx check "$1" -o "$hdr_len"; then
				echo "No valid TRX firmware in the LXL image"
				notify_firmware_test_result "trx_valid" 0
				error=1
			else
				notify_firmware_test_result "trx_valid" 1
			fi
		;;
		"lxlold")
			local board_id=$(dd if="$1" skip=48 bs=1 count=12 2>/dev/null | hexdump -v -e '1/1 "%c"')
			local dev_board_id=$(platform_expected_image)
			echo "Found LXL image with device board_id $board_id"

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
		"safeloader")
		;;
		"seama")
			local img_signature=$(oseama info "$1" | grep "Meta entry:.*signature=" | sed "s/.*=//")
			local dev_signature=$(platform_expected_image)
			echo "Found Seama image with device signature: $img_signature"

			[ -n "$dev_signature" -a "seama $img_signature" != "$dev_signature" ] && {
				echo "Firmware signature doesn't match device signature ($dev_signature)"
				error=1
			}

			$(oseama info "$1" -e 0 | grep -q "Meta entry:.*type=firmware") || {
				echo "Seama container doesn't have firmware entity"
				error=1
			}
		;;
		"trx")
			local expected=$(platform_expected_image)

			[ "$expected" == "safeloader" ] && {
				echo "This device expects SafeLoader format and may not work with TRX"
				error=1
			}

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

# $(1): image for upgrade (with possible extra header)
# $(2): offset of trx in image
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

platform_do_upgrade_nand_seama() {
	local dir="/tmp/sysupgrade-bcm53xx"
	local seama="$1"
	local tmp

	# Extract Seama entity from Seama seal
	rm -fR $dir
	mkdir -p $dir
	oseama extract "$seama" \
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

platform_trx_from_chk_cmd() {
	local header_len=$((0x$(get_magic_long_at "$1" 4)))

	echo -n dd skip=$header_len iflag=skip_bytes
}

platform_trx_from_cybertan_cmd() {
	echo -n dd skip=32 iflag=skip_bytes
}

platform_trx_from_lxl_cmd() {
	local hdr_len=$(get_le_long_at "$1" 8)

	echo -n dd skip=$hdr_len iflag=skip_bytes
}

platform_trx_from_lxlold_cmd() {
	echo -n dd bs=64 skip=1
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

platform_img_from_seama() {
	local dir="/tmp/sysupgrade-bcm53xx"
	local offset=$(oseama info "$1" -e 0 | grep "Entity offset:" | sed "s/.*:\s*//")
	local size=$(oseama info "$1" -e 0 | grep "Entity size:" | sed "s/.*:\s*//")

	# Busybox doesn't support required iflag-s
	# echo -n dd iflag=skip_bytes,count_bytes skip=$offset count=$size

	rm -fR $dir
	mkdir -p $dir
	dd if="$1" of=$dir/image-noheader.bin bs=$offset skip=1
	dd if=$dir/image-noheader.bin of=$dir/image-entity.bin bs=$size count=1

	echo -n $dir/image-entity.bin
}

platform_do_upgrade() {
	local file_type=$(platform_identify "$1")
	local trx="$1"
	local cmd=

	[ "$(platform_flash_type)" == "nand" ] && {
		case "$file_type" in
			"chk")		platform_do_upgrade_nand_trx "$1" $((0x$(get_magic_long_at "$1" 4)));;
			"cybertan")	platform_do_upgrade_nand_trx "$1" 32;;
			"lxl")		platform_do_upgrade_nand_trx "$1" $(get_le_long_at "$1" 8);;
			"lxlold")	platform_do_upgrade_nand_trx "$1" 64;;
			"seama")	platform_do_upgrade_nand_seama "$1";;
			"trx")		platform_do_upgrade_nand_trx "$1";;
		esac

		# Above calls exit on success.
		# If we got here something went wrong.
		echo "Writing whole image to NAND flash. All erase counters will be lost."
	}

	case "$file_type" in
		"chk")		cmd=$(platform_trx_from_chk_cmd "$trx");;
		"cybertan")	cmd=$(platform_trx_from_cybertan_cmd "$trx");;
		"lxl")		cmd=$(platform_trx_from_lxl_cmd "$trx");;
		"lxlold")	cmd=$(platform_trx_from_lxlold_cmd "$trx");;
		"safeloader")	trx=$(platform_img_from_safeloader "$trx"); PART_NAME=os-image;;
		"seama")	trx=$(platform_img_from_seama "$trx");;
	esac

	default_do_upgrade "$trx" "$cmd"
}
