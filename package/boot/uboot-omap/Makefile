#
# Copyright (C) 2012-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=u-boot
PKG_VERSION:=2015.10
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=bdc68d5f9455ad933b059c735d983f2c8b6b552dafb062e5ff1444f623021955

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
  CONFIG:=
  IMAGE:=
endef

define uboot/omap4_panda
  TITLE:=U-Boot for the Pandaboard
endef

define uboot/am335x_evm
  TITLE:=U-Boot for the AM335x EVM
endef

define uboot/omap3_overo
  TITLE:=U-Boot for the Gumstix Overo
endef

define uboot/omap3_beagle
  TITLE:=U-Boot for the BeagleBoard
endef

UBOOTS:=omap4_panda am335x_evm omap3_overo omap3_beagle

define Package/uboot/template
define Package/uboot-omap-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_omap
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
  HIDDEN:=1
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
UBOOT_IMAGE:=$(if $(IMAGE),$(IMAGE),openwrt-$(BOARD)-$(BUILD_VARIANT)-u-boot.img)
endif

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) \
		USE_PRIVATE_LIBGCC=yes $(UBOOT_CONFIG)_config
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS)
endef

define Package/uboot/install/default
	$(INSTALL_DIR) $(BIN_DIR)/uboot-$(BOARD)-$(1)
	$(CP) $(PKG_BUILD_DIR)/u-boot.img $(BIN_DIR)/uboot-$(BOARD)-$(1)/u-boot.img
	$(CP) $(PKG_BUILD_DIR)/MLO $(BIN_DIR)/uboot-$(BOARD)-$(1)/MLO
endef

define Package/uboot/install/template
define Package/uboot-omap-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-omap-$(u))) \
)
