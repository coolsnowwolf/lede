BOARDNAME := Devices which boot from SATA (NAS)
FEATURES += ext4 usb ramdisk
DEFAULT_PACKAGES += badblocks block-mount e2fsprogs \
		    kmod-dm kmod-md-mod partx-utils

define Target/Description
	Build firmware images for APM82181 boards that boot from SATA.
	For NAS like the MyBook Live Series.
endef
