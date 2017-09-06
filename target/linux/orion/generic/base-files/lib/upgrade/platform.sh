#
# Copyright (C) 2010-2011 OpenWrt.org
#

# use default "image" for PART_NAME
# use default for platform_do_upgrade()

platform_check_image() {
	[ "${ARGC}" -gt 1 ] && { echo 'Too many arguments. Only flash file expected.'; return 1; }

	local hardware="$(board_name)"
	local magic="$(get_magic_word "$1")"
	local magic_long="$(get_magic_long "$1")"

	case "${hardware}" in
	 # hardware with a direct uImage partition
	 # image header format as described in U-Boot's include/image.h
	 # see http://git.denx.de/cgi-bin/gitweb.cgi?p=u-boot.git;a=blob;f=include/image.h
	 'Linksys WRT350N v2')
		[ "${magic_long}" != '27051956' ] && {
			echo "Invalid image type ${magic_long}."
			return 1
		}
		return 0
		;;
	 # Netgear WNR854T (has uImage as file inside a JFFS2 partition)
	 'Netgear WNR854T')
		[ "${magic}" != '8519' ] && {
			echo "Invalid image type ${magic}."
			return 1
		}
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on ${hardware}."
	return 1
}
