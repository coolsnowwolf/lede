#
# Copyright (C) 2014 OpenWrt.org
#

SUBTARGET:=raidsonic
BOARDNAME:=Raidsonic NAS42x0
FEATURES+=usb
DEFAULT_PACKAGES+=kmod-usb2 kmod-md-mod kmod-md-linear kmod-md-multipath \
		kmod-md-raid0 kmod-md-raid1 kmod-md-raid10 kmod-md-raid456 \
		kmod-fs-btrfs kmod-fs-cifs kmod-fs-ext4 kmod-fs-nfs \
		kmod-fs-nfsd kmod-fs-ntfs kmod-fs-reiserfs kmod-fs-vfat \
		kmod-nls-utf8 kmod-usb-storage-extras \
		samba36-server mdadm cfdisk fdisk e2fsprogs badblocks

define Target/Description
	Build firmware images for Raidsonic NAS4220.
endef
