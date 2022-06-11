#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=b43legacy-firmware
PKG_VERSION:=3.130.20.0
PKG_RELEASE:=1

PKG_SOURCE:=wl_apsta-$(PKG_VERSION).o
PKG_SOURCE_URL:=@OPENWRT
PKG_HASH:=7dba610b1d96dd14e901bcbce14cd6ecd1b1ac6f5c0035b0d6b6dc46a7c3ef90

include $(INCLUDE_DIR)/package.mk

define Package/b43legacy-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  TITLE:=Broadcom bcm43xx b43legacy firmware
endef

define Package/b43legacy-firmware/config
  if PACKAGE_b43legacy-firmware

	config B43LEGACY_FW_SQUASH
		bool "Remove unnecessary firmware files"
		default y
		help
		  This options allows you to remove unnecessary b43legacy firmware files
		  from the final rootfs image. This can reduce the rootfs size by
		  up to 50k.

		  If unsure, say Y.

	config B43LEGACY_FW_SQUASH_COREREVS
		string "Core revisions to include"
		depends on B43LEGACY_FW_SQUASH
		default "1,2,3,4"
		help
		  This is a comma separated list of core revision numbers.

		  Example (keep files for rev4 only):
		    4

		  Example (keep files for rev2 and rev4):
		    2,4

  endif
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile

endef

define Package/b43legacy-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	b43-fwcutter --unsupported -w $(1)/lib/firmware/ $(DL_DIR)/$(PKG_SOURCE)
  ifneq ($(CONFIG_B43LEGACY_FW_SQUASH),)
	b43-fwsquash.py "G" "$(CONFIG_B43LEGACY_FW_SQUASH_COREREVS)" "$(1)/lib/firmware/b43legacy"
  endif
endef

$(eval $(call BuildPackage,b43legacy-firmware))
