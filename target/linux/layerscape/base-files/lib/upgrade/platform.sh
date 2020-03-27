
#!/bin/sh
#
# Copyright 2015-2019 Traverse Technologies
#
platform_do_upgrade_traverse_nandubi() {
	bootsys=$(fw_printenv bootsys | awk -F= '{{print $2}}')
	newbootsys=2
	if [ "$bootsys" -eq "2" ]; then
		newbootsys=1
	fi

	# If nand_do_upgrade succeeds, we don't have an opportunity to add any actions of
	# our own, so do it here and set back on failure
	echo "Setting bootsys to #${newbootsys}"
	fw_setenv bootsys $newbootsys
	CI_UBIPART="nandubi"
	CI_KERNPART="kernel${newbootsys}"
	CI_ROOTPART="rootfs${newbootsys}"
	nand_do_upgrade "$ARGV" || (echo "Upgrade failed, setting bootsys ${bootsys}" && fw_setenv bootsys $bootsys)

}
platform_check_image() {
	local board=$(board_name)

	case "$board" in
	traverse,ls1043v | \
	traverse,ls1043s)
		nand_do_platform_check "traverse-ls1043" $1
		return $?
		;;
	*)
		echo "Sysupgrade is not currently supported on $board"
		;;
	esac

	return 1
}
platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	traverse,ls1043v | \
	traverse,ls1043s)
		platform_do_upgrade_traverse_nandubi "$ARGV"
		;;
	*)
		echo "Sysupgrade is not currently supported on $board"
		;;
	esac
}
platform_pre_upgrade() {
	# Force the creation of fw_printenv.lock
	mkdir -p /var/lock
	touch /var/lock/fw_printenv.lock

	export RAMFS_COPY_BIN="/usr/sbin/fw_printenv /usr/sbin/fw_setenv /usr/sbin/ubinfo /bin/echo ${RAMFS_COPY_BIN}"
	export RAMFS_COPY_DATA="/etc/fw_env.config /var/lock/fw_printenv.lock ${RAMFS_COPY_DATA}"
}
