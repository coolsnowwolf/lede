PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	avm,fritz3370-rev2-hynix|\
	avm,fritz3370-rev2-micron|\
	avm,fritz7362sl|\
	avm,fritz7412|\
	bt,homehub-v2b|\
	bt,homehub-v3a|\
	bt,homehub-v5a|\
	zyxel,p-2812hnu-f1|\
	zyxel,p-2812hnu-f3)
		nand_do_upgrade $1
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
