# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2020.04
PKG_RELEASE:=$(AUTORELEASE)

PKG_HASH:=fe732aaf037d9cc3c0909bad8362af366ae964bbdac6913a34081ff4ad565372

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=ramips
  UBOOT_IMAGE:=u-boot.bin
endef

define U-Boot/ravpower_rp-wd009
  BUILD_DEVICES:=ravpower_rp-wd009
  BUILD_SUBTARGET:=mt76x8
  NAME:=RAVPower RP-WD009
  UBOOT_CONFIG:=ravpower-rp-wd009-ram
endef

UBOOT_TARGETS := \
	ravpower_rp-wd009

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(VARIANT)-$(UBOOT_IMAGE)
endef

$(eval $(call BuildPackage/U-Boot))
