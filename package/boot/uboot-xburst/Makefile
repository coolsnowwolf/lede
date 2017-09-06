#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=u-boot
PKG_VERSION:=2012.10-rc2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=6d094cafa7ecea8b671fbdcd21130b6a4f5744fc47dd263e101ed5d3629dffd4
PKG_TARGETS:=bin

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
  CONFIG:=
  IMAGE:=
endef

define uboot/qi_lb60
  TITLE:=U-boot for the qi_lb60 board
endef

UBOOTS:=qi_lb60

define Package/uboot/template
define Package/uboot-xburst-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_xburst
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/UBoot/WebHome
  VARIANT:=$(1)
endef
endef

define BuildUbootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	$(call Package/uboot/template,$(1),$(TITLE))
endef


ifdef BUILD_VARIANT
$(eval $(call uboot/$(BUILD_VARIANT)))
UBOOT_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
UBOOT_IMAGE:=$(if $(IMAGE),$(IMAGE),openwrt-$(BOARD)-$(BUILD_VARIANT)-u-boot.bin)
endif

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(UBOOT_CONFIG)_config
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS)
endef

define Package/uboot/install/template
define Package/uboot-xburst-$(1)/install
	$(CP) $(PKG_BUILD_DIR)/u-boot-xburst.bin $(BIN_DIR)/$(2)
	rmdir $$(1)
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),openwrt-$(BOARD)-$(u)-u-boot.bin)) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUbootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-xburst-$(u))) \
)
