#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=u-boot
PKG_VERSION:=2014.10
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot

PKG_HASH:=d3b132a7a9b3f3182b7aad71c2dfbd4fc15bea83e12c76134eb3ffefc07d1c71

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
  CONFIG:=
  IMAGE:=
endef

define uboot/ox820
  TITLE:=U-Boot for the Oxford/PLX NAS7820
endef

UBOOTS:=ox820

define Package/uboot/template
define Package/uboot-oxnas-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_oxnas
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
  MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
endef
endef

define BuildUBootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	$(call Package/uboot/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call uboot/$(BUILD_VARIANT)))
UBOOT_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
UBOOT_IMAGE:=$(if $(IMAGE),$(IMAGE),openwrt-$(BOARD)-$(BUILD_VARIANT)-u-boot.bin)
endif

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) ./files/* $(PKG_BUILD_DIR)
	find $(PKG_BUILD_DIR) -name .svn | $(XARGS) rm -rf
endef

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(UBOOT_CONFIG)_config
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		u-boot.bin \
		CROSS_COMPILE=$(TARGET_CROSS)
endef

define Package/uboot/install/default
	$(INSTALL_DIR) $(BIN_DIR)
	$(CP) $(PKG_BUILD_DIR)/u-boot.bin \
		$(BIN_DIR)/openwrt-$(BOARD)-$(1)-u-boot.bin
	$(CP) $(PKG_BUILD_DIR)/u-boot.bin \
		$(KERNEL_BUILD_DIR)/u-boot.bin
endef

define Package/uboot/install/template
define Package/uboot-oxnas-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-oxnas-$(u))) \
)
