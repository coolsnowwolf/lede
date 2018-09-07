#
# Copyright (C) 2007,2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#


define Image/cmdline/yaffs2
	root=/dev/mtdblock3 rootfstype=yaffs2
endef

define Image/BuildKernel/RouterBoard
	$(CP) $(KDIR)/vmlinux-initramfs.elf $(call imgname,kernel-initramfs,rb1xx)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(call imgname,kernel-initramfs,rb1xx) \
		'$(strip $(call Image/cmdline/yaffs2))'
endef

ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
  define Image/BuildKernel
	$(call Image/BuildKernel/RouterBoard)
  endef
endif

