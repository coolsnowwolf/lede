#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=apex
PKG_VERSION:=1.6.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.openwrt.org/sources/
PKG_HASH:=1d2bc04c2c6bb3d2d6c1916b6dc559cda2b1ecb045d7801fd49af6af4234abeb
PKG_TARGETS:=bin

include $(INCLUDE_DIR)/package.mk

export GCC_HONOUR_COPTS=s

define Package/apex
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_ixp4xx
  DEFAULT:=y
  TITLE:=Boot loader for NSLU2, FSG3, NAS100D and others
  URL:=http://wiki.buici.com/wiki/Apex_Bootloader
endef

define build_apex
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH=arm \
		$(1)_config
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		KBUILD_HAVE_NLS=no \
		ARCH=arm \
		clean all
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/apex.bin $(PKG_BUILD_DIR)/out/apex-$(2).bin
endef

define Build/Compile
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/out
	$(call build_apex,slugos-nslu2-armeb,nslu2-armeb)
	$(call build_apex,slugos-nslu2-16mb-armeb,nslu2-16mb-armeb)
	$(call build_apex,slugos-fsg3-armeb,fsg3-armeb)
	$(call build_apex,slugos-nas100d-armeb,nas100d-armeb)
endef

define Package/apex/install
	$(INSTALL_DIR) $(STAGING_DIR)/apex
	$(CP) $(PKG_BUILD_DIR)/out/*.bin $(1)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/out/*.bin $(STAGING_DIR_IMAGE)/
endef

$(eval $(call BuildPackage,apex))
