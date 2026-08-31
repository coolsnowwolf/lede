PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'

# The TP-Link bootloader gets its flash layout from "bootargs".
# Use this to our advantage, and:
#  1. Disable the second rootfs (usrimg2)
#  2. Extend the first rootfs (usrimg1) to include more space
#  3. Increase the baudrate to the expected 115200
tplink_sg2xxx_fix_mtdparts() {
	local args
	args="bootargs mtdparts=spi_flash:896K(boot),128K(env),6144K(sys)"
	args="$args,20480K(usrimg1@main),0K(usrimg2)"
	args="$args,4096K(usrappfs),1024K(para)\n"
	args="$args baudrate 115200"

	echo -e "$args" | fw_setenv --script -
}

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	plasmacloud,esx28|\
	plasmacloud,mcx3|\
	plasmacloud,psx8|\
	plasmacloud,psx10|\
	plasmacloud,psx28)
		PART_NAME="inactive"
		platform_do_upgrade_dualboot_plasmacloud "$1"
		;;
	tplink,sg2008p-v1|\
	tplink,sg2008p-v3|\
	tplink,sg2210p-v3)
		tplink_sg2xxx_fix_mtdparts
		default_do_upgrade "$1"
		;;
	linksys,lgs328c|\
	linksys,lgs328mpc-v2|\
	linksys,lgs352c)
		nand_do_upgrade "$1"
		;;
	zyxel,xgs1930-28hp|\
	zyxel,xmg1915-10e|\
	zyxel,xmg1915-10ep|\
	zyxel,xs1930-10|\
	zyxel,xs1930-12f|\
	zyxel,xs1930-12hp)
		PART_NAME="factory"
		default_do_upgrade "$1"
		;;
	*)
		default_do_upgrade "$1"
		;;
	esac
}
