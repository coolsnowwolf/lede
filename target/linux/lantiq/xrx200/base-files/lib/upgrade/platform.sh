PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	arcadyan,vrv9510kwac23|\
	avm,fritz3370-rev2-hynix|\
	avm,fritz3370-rev2-micron|\
	avm,fritz3390|\
	avm,fritz3490|\
	avm,fritz3490-micron|\
	avm,fritz5490|\
	avm,fritz5490-micron|\
	avm,fritz7362sl|\
	avm,fritz7412|\
	avm,fritz7430|\
	avm,fritz7490|\
	avm,fritz7490-micron|\
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
