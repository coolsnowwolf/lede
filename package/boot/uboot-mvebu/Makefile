#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=u-boot
PKG_VERSION:=2016.03
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=\
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=e49337262ecac44dbdeac140f2c6ebd1eba345e0162b0464172e7f05583ed7bb

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
endef

define uboot/clearfog
  TITLE:=U-Boot for SolidRun ClearFog A1
  DEVICE:=armada-388-clearfog
endef

UBOOTS:= \
	clearfog

define Package/uboot/template
define Package/uboot-mvebu-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@(TARGET_DEVICE_mvebu_DEVICE_$(3)||TARGET_mvebu_DEVICE_$(3)||TARGET_mvebu_Default)
  DEFAULT:=m
  HIDDEN:=1
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
  MAINTAINER:=Felix Fietkau <nbd@nbd.name>
endef
endef

define BuildUBootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	$(call Package/uboot/template,$(1),$(TITLE),$(DEVICE))
endef

define Build/Configure
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(BUILD_VARIANT)_config
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		DTC_DIR=$(LINUX_DIR)/scripts/dtc/
endef

define Package/uboot/install/default
	$(INSTALL_DIR) $(BIN_DIR)/uboot-$(BOARD)-$(1)
	$(CP) $(PKG_BUILD_DIR)/u-boot-spl.kwb \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/openwrt-$(BOARD)-$(1)-u-boot-spl.kwb
	$(INSTALL_DIR) $(KERNEL_BUILD_DIR)/
	$(CP) $(PKG_BUILD_DIR)/u-boot-spl.kwb \
		$(KERNEL_BUILD_DIR)/openwrt-$(BOARD)-$(1)-u-boot-spl.kwb
	$(INSTALL_DIR) $(BIN_DIR)/u-boot-kwboot/
	$(CP) $(PKG_BUILD_DIR)/tools/kwboot \
		$(BIN_DIR)/u-boot-kwboot/
endef

define Package/uboot/install/template
define Package/uboot-mvebu-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-mvebu-$(u))) \
)
