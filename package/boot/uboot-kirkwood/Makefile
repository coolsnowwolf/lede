#
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2017.09
PKG_RELEASE:=1

PKG_HASH:=b2d15f2cf5f72e706025cde73d67247c6da8cd35f7e10891eefe7d9095089744

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=kirkwood
  BUILD_DEVICES:=$(patsubst %_second_stage,%,$(1))
endef

define U-Boot/dockstar
  NAME:=Seagate DockStar
endef

define U-Boot/dockstar_second_stage
  NAME:=Seagate DockStar (second stage)
endef

define U-Boot/goflexhome
  NAME:=the Seagate GoFlexHome/GoFlexNet
endef

define U-Boot/ib62x0
  NAME:=RaidSonic ICY BOX NAS6210 and NAS6220
endef

define U-Boot/ib62x0_second_stage
  NAME:=RaidSonic ICY BOX NAS6210 and NAS6220 (second stage)
endef

define U-Boot/iconnect
  NAME:=Iomega iConnect Wireless
endef

define U-Boot/iconnect_second_stage
  NAME:=Iomega iConnect Wireless (second stage)
endef

define U-Boot/nsa310
  NAME:=Zyxel NSA310
endef

define U-Boot/nsa325
  NAME:=Zyxel NSA325v1 and v2
endef

define U-Boot/pogo_e02
  NAME:=Cloud Engines Pogoplug E02
endef

define U-Boot/pogo_e02_second_stage
  NAME:=Cloud Engines Pogoplug E02 (second stage)
endef

define U-Boot/sheevaplug
  NAME:=SheevaPlug
endef

UBOOT_TARGETS := \
	dockstar dockstar_second_stage \
	goflexhome \
	ib62x0 ib62x0_second_stage \
	iconnect iconnect_second_stage \
	nsa310 \
	nsa325 \
	pogo_e02 pogo_e02_second_stage \
	sheevaplug

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
		-n 'U-Boot uImage' \
		-d $(PKG_BUILD_DIR)/u-boot.bin $(PKG_BUILD_DIR)/u-boot.img
endef

define Package/u-boot/install
	$(CP) \
		$(PKG_BUILD_DIR)/u-boot.bin \
		$(PKG_BUILD_DIR)/u-boot.kwb \
		$(PKG_BUILD_DIR)/u-boot.img \
		$(1)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/kwboot $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call BuildPackage/U-Boot))
