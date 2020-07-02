BOARDNAME := Devices which boot from SATA (NAS)
DEVICE_TYPE := nas
FEATURES += ext4 usb ramdisk squashfs rootfs-part boot-part
DEFAULT_PACKAGES += badblocks block-mount e2fsprogs kmod-hwmon-drivetemp \
		    kmod-dm kmod-md-mod partx-utils mkf2fs f2fsck

define Target/Description
	Build firmware images for APM82181 boards that boot from SATA.
	For NAS like the MyBook Live Series.
endef
