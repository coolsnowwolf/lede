RAMFS_COPY_BIN='fitblk fit_check_sign'

REQUIRE_IMAGE_METADATA=1

airoha_require_ubi_layout() {
	local ubidev volume fip_volume

	ubidev="$(nand_find_ubi ubi)"
	for volume in fit fip ubootenv ubootenv2 "$@"; do
		if [ -z "$(nand_find_volume "$ubidev" "$volume")" ]; then
			echo "Required UBI volume $volume is missing; repair the layout from U-Boot recovery."
			return 1
		fi
	done

	fip_volume="$(nand_find_volume "$ubidev" fip)"
	[ "$(cat "/sys/class/ubi/$fip_volume/data_bytes")" -gt 0 ] || {
		echo "The fip volume is empty; install BL31/U-Boot first."
		return 1
	}
}

platform_check_image() {
	[ "$#" -gt 1 ] && return 1

	case "$(board_name)" in
	fiberhome,hg5382a|\
	fiberhome,hg5585f-ct|\
	fiberhome,hg5585f-cu|\
	h3c,hm2004-du|\
	nokia,xg-040g-md-ubi|\
	nokia,xg-040g-tf-ubi)
		fit_check_image "$1"
		return $?
		;;
	esac

	return 0
}

platform_do_upgrade() {
	case "$(board_name)" in
	fiberhome,hg5382a|\
	fiberhome,hg5585f-ct|\
	fiberhome,hg5585f-cu|\
	h3c,hm2004-du)
		airoha_require_ubi_layout factory && fit_do_upgrade "$1"
		;;
	nokia,xg-040g-md-ubi|\
	nokia,xg-040g-tf-ubi)
		airoha_require_ubi_layout bosa ri && fit_do_upgrade "$1"
		;;
	*)
		nand_do_upgrade "$1"
		;;
	esac
}
