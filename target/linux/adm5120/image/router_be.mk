#
# Copyright (C) 2007,2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Image/Build/ZyXEL
	$(call Image/Build/Loader,$(2),bin,0x80500000,0,y,$(2))
	$(call Image/Build/TRXNoloader,$(call imgname,$(1),$(2)).trx,$(1))
	$(STAGING_DIR_HOST)/bin/mkzynfw -B $(2) \
		-b $(KDIR)/loader-$(2).bin \
		-r $(call imgname,$(1),$(2)).trx:0x10000 \
		-o $(call imgname,$(1),$(2))-webui.bin
endef

define Image/Build/Template/ZyXEL
	$(call Image/Build/ZyXEL,$(1),$(2))
endef

define Image/Build/Template/ZyXEL/squashfs
	$(call Image/Build/Template/ZyXEL,squashfs,$(1))
endef

define Image/Build/Template/ZyXEL/jffs2
	$(call Image/Build/Template/ZyXEL,jffs2,$(1))
endef

define Image/Build/Template/ZyXEL/Initramfs
	$(call Image/Build/LZMAKernel/KArgs,$(1),bin)
endef

#
# Profiles
#
define Image/Build/Profile/P334WT
	$(call Image/Build/Template/ZyXEL/$(1),p-334wt)
endef

define Image/Build/Profile/P335WT
	$(call Image/Build/Template/ZyXEL/$(1),p-335wt)
endef

define Image/Build/Profile/Generic
	$(call Image/Build/Profile/P334WT,$(1))
	$(call Image/Build/Profile/P335WT,$(1))
endef

