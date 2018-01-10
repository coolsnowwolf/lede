#
# Copyright (C) 2013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=u-boot
PKG_VERSION:=2014.04
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=7b6444bd23eb61068c43bd1d44ec7e7bfdbce5cadeca20c833eee186b4d3fd31

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
  CONFIG:=
  IMAGE:=
endef

define uboot/mx6qsabresd
  TITLE:=U-Boot for SABRE i.MX6Quad board
endef

define uboot/nitrogen6dl
  TITLE:=U-Boot for Nitrogen6x i.MX6Dual-Lite 1GB board
endef

define uboot/nitrogen6dl2g
  TITLE:=U-Boot for Nitrogen6x i.MX6Dual-Lite 2GB board
endef

define uboot/nitrogen6q
  TITLE:=U-Boot for Nitrogen6x/SABRE Lite (MX6Q/1GB)
endef

define uboot/nitrogen6q2g
  TITLE:=U-Boot for Nitrogen6x i.MX6Quad 2GB board
endef

define uboot/nitrogen6s
  TITLE:=U-Boot for Nitrogen6x i.MX6Solo 512MB board
endef

define uboot/nitrogen6s1g
  TITLE:=U-Boot for Nitrogen6x i.MX6Solo 1GB board
endef

define uboot/wandboard_dl
  TITLE:=U-Boot for the Wandboard Dual Lite
endef

define uboot/wandboard_quad
  TITLE:=U-Boot for the Wandboard Quad
endef

define uboot/wandboard_solo
  TITLE:=U-Boot for the Wandboard Solo
endef

UBOOTS := \
	mx6qsabresd \
	nitrogen6dl \
	nitrogen6dl2g \
	nitrogen6q \
	nitrogen6q2g \
	nitrogen6s \
	nitrogen6s1g \
	wandboard_dl \
	wandboard_quad \
	wandboard_solo

define Package/uboot/template
define Package/uboot-imx6-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_imx6
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
  MAINTAINER:=Luka Perkov <luka@openwrt.org>
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

define Build/Configure
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(UBOOT_CONFIG)_config
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS)
endef

define Package/uboot/install/default
	$(INSTALL_DIR) $(BIN_DIR)/uboot-$(BOARD)-$(1)
	$(CP) \
		$(PKG_BUILD_DIR)/u-boot.imx \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/openwrt-$(BOARD)-$(1)-u-boot.imx
endef

define Package/uboot/install/template
define Package/uboot-imx6-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-imx6-$(u))) \
)
