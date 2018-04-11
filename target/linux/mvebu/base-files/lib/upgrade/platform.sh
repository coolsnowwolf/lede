#
# Copyright (C) 2014-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#

RAMFS_COPY_BIN='fw_printenv fw_setenv'
RAMFS_COPY_DATA='/etc/fw_env.config /var/lock/fw_printenv.lock'
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	return 0
}

platform_do_upgrade() {
	case "$(board_name)" in
	armada-385-linksys-caiman|armada-385-linksys-cobra|armada-385-linksys-rango|armada-385-linksys-shelby|armada-xp-linksys-mamba)
		platform_do_upgrade_linksys "$ARGV"
		;;
	armada-385-turris-omnia|armada-388-clearfog-base|armada-388-clearfog-pro|globalscale,espressobin|marvell,armada8040-mcbin)
		platform_do_upgrade_sdcard "$ARGV"
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}
platform_copy_config() {
	case "$(board_name)" in
	armada-385-linksys-caiman|armada-385-linksys-cobra|armada-385-linksys-rango|armada-385-linksys-shelby|armada-xp-linksys-mamba)
		platform_copy_config_linksys
		;;
	armada-385-turris-omnia|armada-388-clearfog-base|armada-388-clearfog-pro|globalscale,espressobin|marvell,armada8040-mcbin)
		platform_copy_config_sdcard "$ARGV"
		;;
	esac
}
