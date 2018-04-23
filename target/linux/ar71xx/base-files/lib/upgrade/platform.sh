#
# Copyright (C) 2011 OpenWrt.org
#

. /lib/functions/system.sh
. /lib/ar71xx.sh

PART_NAME=firmware
RAMFS_COPY_DATA=/lib/ar71xx.sh
RAMFS_COPY_BIN='nandwrite'

CI_BLKSZ=65536
CI_LDADR=0x80060000

PLATFORM_DO_UPGRADE_COMBINED_SEPARATE_MTD=0

platform_find_partitions() {
	local first dev size erasesize name
	while read dev size erasesize name; do
		name=${name#'"'}; name=${name%'"'}
		case "$name" in
			vmlinux.bin.l7|vmlinux|kernel|linux|linux.bin|rootfs|filesystem)
				if [ -z "$first" ]; then
					first="$name"
				else
					echo "$erasesize:$first:$name"
					break
				fi
			;;
		esac
	done < /proc/mtd
}

platform_find_kernelpart() {
	local part
	for part in "${1%:*}" "${1#*:}"; do
		case "$part" in
			vmlinux.bin.l7|vmlinux|kernel|linux|linux.bin)
				echo "$part"
				break
			;;
		esac
	done
}

platform_find_rootfspart() {
	local part
	for part in "${1%:*}" "${1#*:}"; do
		[ "$part" != "$2" ] && echo "$part" && break
	done
}

platform_do_upgrade_combined() {
	local partitions=$(platform_find_partitions)
	local kernelpart=$(platform_find_kernelpart "${partitions#*:}")
	local erase_size=$((0x${partitions%%:*})); partitions="${partitions#*:}"
	local kern_length=0x$(dd if="$1" bs=2 skip=1 count=4 2>/dev/null)
	local kern_blocks=$(($kern_length / $CI_BLKSZ))
	local root_blocks=$((0x$(dd if="$1" bs=2 skip=5 count=4 2>/dev/null) / $CI_BLKSZ))

	if [ -n "$partitions" ] && [ -n "$kernelpart" ] && \
	   [ ${kern_blocks:-0} -gt 0 ] && \
	   [ ${root_blocks:-0} -gt 0 ] && \
	   [ ${erase_size:-0} -gt 0 ];
	then
		local rootfspart=$(platform_find_rootfspart "$partitions" "$kernelpart")
		local append=""
		[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

		if [ "$PLATFORM_DO_UPGRADE_COMBINED_SEPARATE_MTD" -ne 1 ]; then
		    ( dd if="$1" bs=$CI_BLKSZ skip=1 count=$kern_blocks 2>/dev/null; \
		      dd if="$1" bs=$CI_BLKSZ skip=$((1+$kern_blocks)) count=$root_blocks 2>/dev/null ) | \
			    mtd -r $append -F$kernelpart:$kern_length:$CI_LDADR,rootfs write - $partitions
		elif [ -n "$rootfspart" ]; then
		    dd if="$1" bs=$CI_BLKSZ skip=1 count=$kern_blocks 2>/dev/null | \
			    mtd write - $kernelpart
		    dd if="$1" bs=$CI_BLKSZ skip=$((1+$kern_blocks)) count=$root_blocks 2>/dev/null | \
			    mtd -r $append write - $rootfspart
		fi
	fi
	PLATFORM_DO_UPGRADE_COMBINED_SEPARATE_MTD=0
}

tplink_get_image_hwid() {
	get_image "$@" | dd bs=4 count=1 skip=16 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_get_image_mid() {
	get_image "$@" | dd bs=4 count=1 skip=17 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_get_image_boot_size() {
	get_image "$@" | dd bs=4 count=1 skip=37 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

tplink_pharos_check_support_list() {
	local image="$1"
	local offset="$2"
	local model="$3"

	# Here $image is given to dd directly instead of using get_image;
	# otherwise the skip will take almost a second (as dd can't seek)
	dd if="$image" bs=1 skip=$offset count=1024 2>/dev/null | (
		while IFS= read -r line; do
			[ "$line" = "$model" ] && exit 0
		done

		exit 1
	)
}

tplink_pharos_check_image() {
	local magic_long="$(get_magic_long "$1")"
	[ "$magic_long" != "7f454c46" ] && {
		echo "Invalid image magic '$magic_long'"
		return 1
	}

	local model_string="$(tplink_pharos_get_model_string)"

	# New images have the support list at 7802888, old ones at 1511432
	tplink_pharos_check_support_list "$1" 7802888 "$model_string" || \
	tplink_pharos_check_support_list "$1" 1511432 "$model_string" || {
		echo "Unsupported image (model not in support-list)"
		return 1
	}

	return 0
}

seama_get_type_magic() {
	get_image "$@" | dd bs=1 count=4 skip=53 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

wrgg_get_image_magic() {
	get_image "$@" | dd bs=4 count=1 skip=8 2>/dev/null | hexdump -v -n 4 -e '1/1 "%02x"'
}

cybertan_get_image_magic() {
	get_image "$@" | dd bs=8 count=1 skip=0  2>/dev/null | hexdump -v -n 8 -e '1/1 "%02x"'
}

cybertan_check_image() {
	local magic="$(cybertan_get_image_magic "$1")"
	local fw_magic="$(cybertan_get_hw_magic)"

	[ "$fw_magic" != "$magic" ] && {
		echo "Invalid image, ID mismatch, got:$magic, but need:$fw_magic"
		return 1
	}

	return 0
}

platform_do_upgrade_compex() {
	local fw_file=$1
	local fw_part=$PART_NAME
	local fw_mtd=$(find_mtd_part $fw_part)
	local fw_length=0x$(dd if="$fw_file" bs=2 skip=1 count=4 2>/dev/null)
	local fw_blocks=$(($fw_length / 65536))

	if [ -n "$fw_mtd" ] &&  [ ${fw_blocks:-0} -gt 0 ]; then
		local append=""
		[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

		sync
		dd if="$fw_file" bs=64k skip=1 count=$fw_blocks 2>/dev/null | \
			mtd $append write - "$fw_part"
	fi
}

alfa_check_image() {
	local magic_long="$(get_magic_long "$1")"
	local fw_part_size=$(mtd_get_part_size firmware)

	case "$magic_long" in
	"27051956")
		[ "$fw_part_size" != "16318464" ] && {
			echo "Invalid image magic \"$magic_long\" for $fw_part_size bytes"
			return 1
		}
		;;
	"68737173")
		[ "$fw_part_size" != "7929856" ] && {
			echo "Invalid image magic \"$magic_long\" for $fw_part_size bytes"
			return 1
		}
		;;
	esac

	return 0
}

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_word "$1")"
	local magic_long="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	airgateway|\
	airgatewaypro|\
	airrouter|\
	ap121f|\
	ap132|\
	ap531b0|\
	ap90q|\
	archer-c25-v1|\
	archer-c58-v1|\
	archer-c59-v1|\
	archer-c60-v1|\
	archer-c60-v2|\
	archer-c7-v4|\
	bullet-m|\
	c-55|\
	carambola2|\
	cf-e316n-v2|\
	cf-e320n-v2|\
	cf-e355ac-v1|\
	cf-e355ac-v2|\
	cf-e375ac|\
	cf-e380ac-v1|\
	cf-e380ac-v2|\
	cf-e385ac|\
	cf-e520n|\
	cf-e530n|\
	cpe505n|\
	cpe830|\
	cpe870|\
	dap-1330-a1|\
	dgl-5500-a1|\
	dhp-1565-a1|\
	dir-505-a1|\
	dir-600-a1|\
	dir-615-c1|\
	dir-615-e1|\
	dir-615-e4|\
	dir-615-i1|\
	dir-825-c1|\
	dir-835-a1|\
	dlan-hotspot|\
	dlan-pro-1200-ac|\
	dlan-pro-500-wp|\
	dr342|\
	dr531|\
	dragino2|\
	e1700ac-v2|\
	e600g-v2|\
	e600gac-v2|\
	ebr-2310-c1|\
	ens202ext|\
	epg5000|\
	esr1750|\
	esr900|\
	ew-balin|\
	ew-dorin|\
	ew-dorin-router|\
	gl-ar150|\
	gl-ar300m|\
	gl-ar300|\
	gl-ar750|\
	gl-domino|\
	gl-mifi|\
	gl-usb150|\
	hiwifi-hc6361|\
	hornet-ub-x2|\
	jwap230|\
	lbe-m5|\
	lima|\
	loco-m-xw|\
	mzk-w04nu|\
	mzk-w300nh|\
	n5q|\
	nanostation-m|\
	nanostation-m-xw|\
	nbg460n_550n_550nh|\
	pqi-air-pen|\
	r36a|\
	r602n|\
	rme-eg200|\
	rocket-m|\
	rocket-m-ti|\
	rocket-m-xw|\
	rw2458n|\
	sc1750|\
	sc300m|\
	sc450|\
	sr3200|\
	t830|\
	tew-632brp|\
	tew-712br|\
	tew-732br|\
	tew-823dru|\
	tl-wr1043n-v5|\
	tl-wr942n-v1|\
	unifi|\
	unifi-outdoor|\
	unifiac-lite|\
	unifiac-pro|\
	wam250|\
	weio|\
	whr-g301n|\
	whr-hp-g300n|\
	whr-hp-gn|\
	wlae-ag300n|\
	wndap360|\
	wpj342|\
	wpj344|\
	wpj531|\
	wpj558|\
	wpj563|\
	wrt400n|\
	wrtnode2q|\
	wzr-450hp2|\
	wzr-hp-ag300h|\
	wzr-hp-g300nh|\
	wzr-hp-g300nh2|\
	wzr-hp-g450h|\
	xd3200)
		[ "$magic" != "2705" ] && {
			echo "Invalid image type."
			return 1
		}

		return 0
		;;
	alfa-ap96|\
	alfa-nx|\
	ap121|\
	ap121-mini|\
	ap135-020|\
	ap136-010|\
	ap136-020|\
	ap147-010|\
	ap152|\
	ap91-5g|\
	ap96|\
	arduino-yun|\
	bhr-4grv2|\
	bxu2000n-2-a1|\
	db120|\
	dr344|\
	dw33d|\
	f9k1115v2|\
	hornet-ub|\
	mr12|\
	mr16|\
	zbt-we1526|\
	zcn-1523h-2|\
	zcn-1523h-5)
		[ "$magic_long" != "68737173" -a "$magic_long" != "19852003" ] && {
			echo "Invalid image type."
			return 1
		}

		return 0
		;;
	all0258n|\
	all0315n|\
	cap324|\
	cap4200ag|\
	cr3000|\
	cr5000)
		platform_check_image_allnet "$1" && return 0
		return 1
		;;
	all0305|\
	eap300v2|\
	eap7660d|\
	ja76pf|\
	ja76pf2|\
	jwap003|\
	ls-sr71|\
	pb42|\
	pb44|\
	routerstation|\
	routerstation-pro|\
	wp543|\
	wpe72)
		[ "$magic" != "4349" ] && {
			echo "Invalid image. Use *-sysupgrade.bin files on this board"
			return 1
		}

		local md5_img=$(dd if="$1" bs=2 skip=9 count=16 2>/dev/null)
		local md5_chk=$(dd if="$1" bs=$CI_BLKSZ skip=1 2>/dev/null | md5sum -); md5_chk="${md5_chk%% *}"

		if [ -n "$md5_img" -a -n "$md5_chk" ] && [ "$md5_img" = "$md5_chk" ]; then
			return 0
		else
			echo "Invalid image. Contents do not match checksum (image:$md5_img calculated:$md5_chk)"
			return 1
		fi

		return 0
		;;
	antminer-s1|\
	antminer-s3|\
	antrouter-r1|\
	archer-c5|\
	archer-c7|\
	el-m150|\
	el-mini|\
	gl-inet|\
	lan-turtle|\
	mc-mac1200r|\
	minibox-v1|\
	omy-g1|\
	omy-x1|\
	onion-omega|\
	oolite-v1|\
	oolite-v5.2|\
	oolite-v5.2-dev|\
	packet-squirrel|\
	re355|\
	re450|\
	rut900|\
	smart-300|\
	som9331|\
	tellstick-znet-lite|\
	tl-mr10u|\
	tl-mr11u|\
	tl-mr12u|\
	tl-mr13u|\
	tl-mr3020|\
	tl-mr3040|\
	tl-mr3040-v2|\
	tl-mr3220|\
	tl-mr3220-v2|\
	tl-mr3420|\
	tl-mr3420-v2|\
	tl-mr6400|\
	tl-wa701nd-v2|\
	tl-wa7210n-v2|\
	tl-wa750re|\
	tl-wa7510n|\
	tl-wa801nd-v2|\
	tl-wa801nd-v3|\
	tl-wa830re-v2|\
	tl-wa850re|\
	tl-wa850re-v2|\
	tl-wa855re-v1|\
	tl-wa860re|\
	tl-wa901nd|\
	tl-wa901nd-v2|\
	tl-wa901nd-v3|\
	tl-wa901nd-v4|\
	tl-wa901nd-v5|\
	tl-wdr3320-v2|\
	tl-wdr3500|\
	tl-wdr4300|\
	tl-wdr4900-v2|\
	tl-wdr6500-v2|\
	tl-wpa8630|\
	tl-wr1041n-v2|\
	tl-wr1043nd|\
	tl-wr1043nd-v2|\
	tl-wr1043nd-v4|\
	tl-wr2543n|\
	tl-wr703n|\
	tl-wr710n|\
	tl-wr720n-v3|\
	tl-wr740n-v6|\
	tl-wr741nd|\
	tl-wr741nd-v4|\
	tl-wr802n-v1|\
	tl-wr802n-v2|\
	tl-wr810n|\
	tl-wr810n-v2|\
	tl-wr840n-v2|\
	tl-wr840n-v3|\
	tl-wr841n-v1|\
	tl-wr841n-v7|\
	tl-wr841n-v8|\
	tl-wr841n-v9|\
	tl-wr841n-v11|\
	tl-wr842n-v2|\
	tl-wr842n-v3|\
	tl-wr902ac-v1|\
	tl-wr940n-v4|\
	tl-wr940n-v6|\
	tl-wr941nd|\
	tl-wr941nd-v5|\
	tl-wr941nd-v6|\
	wifi-pineapple-nano)
		local magic_ver="0100"

		case "$board" in
		tl-wdr6500-v2)
			magic_ver="0200"
			;;
		esac

		[ "$magic" != "$magic_ver" ] && {
			echo "Invalid image type."
			return 1
		}

		local hwid
		local mid
		local imagehwid
		local imagemid

		hwid=$(tplink_get_hwid)
		mid=$(tplink_get_mid)
		imagehwid=$(tplink_get_image_hwid "$1")
		imagemid=$(tplink_get_image_mid "$1")

		[ "$hwid" != "$imagehwid" -o "$mid" != "$imagemid" ] && {
			echo "Invalid image, hardware ID mismatch, hw:$hwid $mid image:$imagehwid $imagemid."
			return 1
		}

		local boot_size

		boot_size=$(tplink_get_image_boot_size "$1")
		[ "$boot_size" != "00000000" ] && {
			echo "Invalid image, it contains a bootloader."
			return 1
		}

		return 0
		;;
	bsb|\
	dir-825-b1|\
	tew-673gru)
		dir825b_check_image "$1" && return 0
		;;
	rb-411|\
	rb-411u|\
	rb-433|\
	rb-433u|\
	rb-435g|\
	rb-450|\
	rb-450g|\
	rb-493|\
	rb-493g|\
	rb-750|\
	rb-750gl|\
	rb-751|\
	rb-751g|\
	rb-911g-2hpnd|\
	rb-911g-5hpnd|\
	rb-911g-5hpacd|\
	rb-912uag-2hpnd|\
	rb-912uag-5hpnd|\
	rb-921gs-5hpacd-r2|\
	rb-951g-2hnd|\
	rb-951ui-2hnd|\
	rb-2011l|\
	rb-2011il|\
	rb-2011uas|\
	rb-2011uas-2hnd|\
	rb-2011uias|\
	rb-2011uias-2hnd|\
	rb-sxt2n|\
	rb-sxt5n)
		nand_do_platform_check routerboard $1
		return $?
		;;
	c-60|\
	hiveap-121|\
	nbg6716|\
	r6100|\
	rambutan|\
	wi2a-ac200i|\
	wndr3700v4|\
	wndr4300)
		nand_do_platform_check $board $1
		return $?
		;;
	cpe210|\
	cpe510|\
	eap120|\
	wbs210|\
	wbs510)
		tplink_pharos_check_image "$1" && return 0
		return 1
		;;
	a40|\
	a60|\
	mr1750|\
	mr1750v2|\
	mr600|\
	mr600v2|\
	mr900|\
	mr900v2|\
	om2p|\
	om2p-hs|\
	om2p-hsv2|\
	om2p-hsv3|\
	om2p-hsv4|\
	om2p-lc|\
	om2pv2|\
	om2pv4|\
	om5p|\
	om5p-ac|\
	om5p-acv2|\
	om5p-an)
		platform_check_image_openmesh "$magic_long" "$1" && return 0
		return 1
		;;
	mr18|\
	z1)
		merakinand_do_platform_check $board $1
		return $?
		;;
	dir-869-a1|\
	mynet-n600|\
	mynet-n750|\
	qihoo-c301)
		[ "$magic_long" != "5ea3a417" ] && {
			echo "Invalid image, bad magic: $magic_long"
			return 1
		}

		local typemagic=$(seama_get_type_magic "$1")
		[ "$typemagic" != "6669726d" ] && {
			echo "Invalid image, bad type: $typemagic"
			return 1
		}

		return 0
		;;
	e2100l|\
	mynet-rext|\
	wrt160nl)
		cybertan_check_image "$1" && return 0
		return 1
		;;
	nbg6616|\
	uap-pro|\
	unifi-outdoor-plus)
		[ "$magic_long" != "19852003" ] && {
			echo "Invalid image type."
			return 1
		}

		return 0
		;;
	tube2h)
		alfa_check_image "$1" && return 0
		return 1
		;;
	wndr3700|\
	wnr1000-v2|\
	wnr2000-v3|\
	wnr612-v2|\
	wpn824n)
		local hw_magic

		hw_magic="$(ar71xx_get_mtd_part_magic firmware)"
		[ "$magic_long" != "$hw_magic" ] && {
			echo "Invalid image, hardware ID mismatch, hw:$hw_magic image:$magic_long."
			return 1
		}

		return 0
		;;
	wnr2000-v4)
		[ "$magic_long" != "32303034" ] && {
			echo "Invalid image type."
			return 1
		}

		return 0
		;;
	wnr2200)
		[ "$magic_long" != "32323030" ] && {
			echo "Invalid image type."
			return 1
		}

		return 0
		;;
	dap-2695-a1)
		local magic=$(wrgg_get_image_magic "$1")
		[ "$magic" != "21030820" ] && {
			echo "Invalid image, bad type: $magic"
			return 1
		}

		return 0;
		;;
	# these boards use metadata images
	fritz300e|\
	rb-750-r2|\
	rb-750p-pbr2|\
	rb-750up-r2|\
	rb-911-2hn|\
	rb-911-5hn|\
	rb-941-2nd|\
	rb-951ui-2nd|\
	rb-952ui-5ac2nd|\
	rb-962uigs-5hact2hnt|\
	rb-lhg-5nd|\
	rb-map-2nd|\
	rb-mapl-2nd|\
	rb-wap-2nd|\
	rb-wapg-5hact2hnd)
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	rb-750-r2|\
	rb-750p-pbr2|\
	rb-750up-r2|\
	rb-911-2hn|\
	rb-911-5hn|\
	rb-941-2nd|\
	rb-951ui-2nd|\
	rb-952ui-5ac2nd|\
	rb-962uigs-5hact2hnt|\
	rb-lhg-5nd|\
	rb-map-2nd|\
	rb-mapl-2nd|\
	rb-wap-2nd|\
	rb-wapg-5hact2hnd)
		# erase firmware if booted from initramfs
		[ -z "$(rootfs_type)" ] && mtd erase firmware
		;;
	esac
}

platform_nand_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	rb*)
		CI_KERNPART=none
		local fw_mtd=$(find_mtd_part kernel)
		fw_mtd="${fw_mtd/block/}"
		[ -n "$fw_mtd" ] || return
		mtd erase kernel
		tar xf "$1" sysupgrade-routerboard/kernel -O | nandwrite -o "$fw_mtd" -
		;;
	wi2a-ac200i)
		case "$(fw_printenv -n dualPartition)" in
			imgA)
				fw_setenv dualPartition imgB
				fw_setenv ActImg NokiaImageB
			;;
			imgB)
				fw_setenv dualPartition imgA
				fw_setenv ActImg NokiaImageA
			;;
		esac
		ubiblock -r /dev/ubiblock0_0 2>/dev/null >/dev/null
		rm -f /dev/ubiblock0_0
		ubidetach -d 0 2>/dev/null >/dev/null
		CI_UBIPART=ubi_alt
		CI_KERNPART=kernel_alt
		;;
	esac
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	all0258n)
		platform_do_upgrade_allnet "0x9f050000" "$ARGV"
		;;
	all0305|\
	eap7660d|\
	ja76pf|\
	ja76pf2|\
	jwap003|\
	ls-sr71|\
	pb42|\
	pb44|\
	routerstation|\
	routerstation-pro)
		platform_do_upgrade_combined "$ARGV"
		;;
	all0315n)
		platform_do_upgrade_allnet "0x9f080000" "$ARGV"
		;;
	cap4200ag|\
	eap300v2|\
	ens202ext)
		platform_do_upgrade_allnet "0xbf0a0000" "$ARGV"
		;;
	dir-825-b1|\
	tew-673gru)
		platform_do_upgrade_dir825b "$ARGV"
		;;
	a40|\
	a60|\
	mr1750|\
	mr1750v2|\
	mr600|\
	mr600v2|\
	mr900|\
	mr900v2|\
	om2p|\
	om2p-hs|\
	om2p-hsv2|\
	om2p-hsv3|\
	om2p-hsv4|\
	om2p-lc|\
	om2pv2|\
	om2pv4|\
	om5p|\
	om5p-ac|\
	om5p-acv2|\
	om5p-an)
		platform_do_upgrade_openmesh "$ARGV"
		;;
	c-60|\
	hiveap-121|\
	nbg6716|\
	r6100|\
	rambutan|\
	rb-411|\
	rb-411u|\
	rb-433|\
	rb-433u|\
	rb-435g|\
	rb-450|\
	rb-450g|\
	rb-493|\
	rb-493g|\
	rb-750|\
	rb-750gl|\
	rb-751|\
	rb-751g|\
	rb-911g-2hpnd|\
	rb-911g-5hpacd|\
	rb-911g-5hpnd|\
	rb-912uag-2hpnd|\
	rb-912uag-5hpnd|\
	rb-921gs-5hpacd-r2|\
	rb-951g-2hnd|\
	rb-951ui-2hnd|\
	rb-2011il|\
	rb-2011l|\
	rb-2011uas|\
	rb-2011uas-2hnd|\
	rb-2011uias|\
	rb-2011uias-2hnd|\
	rb-sxt2n|\
	rb-sxt5n|\
	wi2a-ac200i|\
	wndr3700v4|\
	wndr4300)
		nand_do_upgrade "$1"
		;;
	mr18|\
	z1)
		merakinand_do_upgrade "$1"
		;;
	uap-pro|\
	unifi-outdoor-plus)
		MTD_CONFIG_ARGS="-s 0x180000"
		default_do_upgrade "$ARGV"
		;;
	wp543|\
	wpe72)
		platform_do_upgrade_compex "$ARGV"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}
