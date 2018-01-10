#
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=u-boot
PKG_VERSION:=2016.09.01
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=\
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=95728e89dd476d17428f94080752ab48884be477b6a678941582aeef618b70bb
PKG_TARGETS:=bin

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
endef

define uboot/dockstar
  TITLE:=U-Boot for Seagate DockStar
endef

define uboot/dockstar_second_stage
  TITLE:=second stage U-Boot for Seagate DockStar
endef

define uboot/goflexhome
  TITLE:=U-Boot for the Seagate GoFlexHome/GoFlexNet
endef

define uboot/ib62x0
  TITLE:=U-Boot for RaidSonic ICY BOX NAS6210 and NAS6220
endef

define uboot/ib62x0_second_stage
  TITLE:=second stage U-Boot for RaidSonic ICY BOX NAS6210 and NAS6220
endef

define uboot/iconnect
  TITLE:=U-Boot for Iomega iConnect Wireless
endef

define uboot/iconnect_second_stage
  TITLE:=second stage U-Boot for Iomega iConnect Wireless
endef

define uboot/pogo_e02
  TITLE:=U-Boot for Cloud Engines Pogoplug E02
endef

define uboot/pogo_e02_second_stage
  TITLE:=second stage U-Boot for Cloud Engines Pogoplug E02
endef

define uboot/sheevaplug
  TITLE:=U-Boot for SheevaPlug
endef

UBOOTS:= \
	dockstar dockstar_second_stage \
	goflexhome \
	ib62x0 ib62x0_second_stage \
	iconnect iconnect_second_stage \
	pogo_e02 pogo_e02_second_stage \
	sheevaplug

define Package/uboot/template
define Package/uboot-kirkwood-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_kirkwood
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
endef
endef

define BuildUBootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	$(call Package/uboot/template,$(1),$(TITLE))
endef

define Build/Configure
	$(if $(findstring _second_stage,$(BUILD_VARIANT)),
		$(CP) \
			$(PKG_BUILD_DIR)/configs/$(subst _second_stage,,$(BUILD_VARIANT))_defconfig \
			$(PKG_BUILD_DIR)/configs/$(BUILD_VARIANT)_defconfig
		echo CONFIG_SECOND_STAGE=y >> $(PKG_BUILD_DIR)/configs/$(BUILD_VARIANT)_defconfig
	)
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(BUILD_VARIANT)_config V=1
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		u-boot.kwb \
		CROSS_COMPILE=$(TARGET_CROSS)
	mkimage -A $(ARCH) -O linux -T kernel -C none \
		-a 0x600000 -e 0x600000 \
		-n 'OpenWrt Das U-Boot uImage' \
		-d $(PKG_BUILD_DIR)/u-boot.bin $(PKG_BUILD_DIR)/u-boot.img
endef

define Package/uboot/install/default
	$(INSTALL_DIR) $(BIN_DIR)/uboot-$(BOARD)-$(1)
	$(CP) $(PKG_BUILD_DIR)/u-boot.bin \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/openwrt-$(BOARD)-$(1)-u-boot.bin
	$(CP) $(PKG_BUILD_DIR)/u-boot.kwb \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/openwrt-$(BOARD)-$(1)-u-boot.kwb
	$(CP) $(PKG_BUILD_DIR)/u-boot.img \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/openwrt-$(BOARD)-$(1)-u-boot.img
	$(INSTALL_DIR) $(BIN_DIR)/u-boot-kwboot/
	$(CP) $(PKG_BUILD_DIR)/tools/kwboot \
		$(BIN_DIR)/u-boot-kwboot/
endef

define Package/uboot/install/template
define Package/uboot-kirkwood-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-kirkwood-$(u))) \
)
