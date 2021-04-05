#
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2020.04
PKG_RELEASE:=1

PKG_HASH:=fe732aaf037d9cc3c0909bad8362af366ae964bbdac6913a34081ff4ad565372

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=kirkwood
endef

define U-Boot/dockstar
  NAME:=Seagate DockStar
  BUILD_DEVICES:=seagate_dockstar
endef

define U-Boot/dockstar_second_stage
  NAME:=Seagate DockStar (second stage)
  BUILD_DEVICES:=seagate_dockstar
endef

define U-Boot/goflexhome
  NAME:=the Seagate GoFlexHome/GoFlexNet
  BUILD_DEVICES:=seagate_goflexhome seagate_goflexnet
endef

define U-Boot/ib62x0
  NAME:=RaidSonic ICY BOX NAS6210 and NAS6220
  BUILD_DEVICES:=raidsonic_ib-nas62x0
endef

define U-Boot/ib62x0_second_stage
  NAME:=RaidSonic ICY BOX NAS6210 and NAS6220 (second stage)
  BUILD_DEVICES:=raidsonic_ib-nas62x0
endef

define U-Boot/iconnect
  NAME:=Iomega iConnect Wireless
  BUILD_DEVICES:=iom_iconnect-1.1
endef

define U-Boot/iconnect_second_stage
  NAME:=Iomega iConnect Wireless (second stage)
  BUILD_DEVICES:=iom_iconnect-1.1
endef

define U-Boot/l-50
  NAME:=CheckPoint L-50
  BUILD_DEVICES:=checkpoint_l-50
endef

define U-Boot/nas220
  NAME:=Seagate Blackarmor NAS220
  BUILD_DEVICES:=seagate_blackarmor-nas220
endef

define U-Boot/nsa310
  NAME:=Zyxel NSA310
  BUILD_DEVICES:=zyxel_nsa310b
endef

define U-Boot/nsa310s
  NAME:=Zyxel NSA310S
  BUILD_DEVICES:=zyxel_nsa310s
endef

define U-Boot/nsa325
  NAME:=Zyxel NSA325v1 and v2
  BUILD_DEVICES:=zyxel_nsa325
endef

define U-Boot/pogo_e02
  NAME:=Cloud Engines Pogoplug E02
  BUILD_DEVICES:=cloudengines_pogoe02
endef

define U-Boot/pogo_e02_second_stage
  NAME:=Cloud Engines Pogoplug E02 (second stage)
  BUILD_DEVICES:=cloudengines_pogoe02
endef

define U-Boot/pogoplugv4
  NAME:=Cloud Engines Pogoplug V4
  BUILD_DEVICES:=cloudengines_pogoplugv4
endef

UBOOT_TARGETS := \
	dockstar dockstar_second_stage \
	goflexhome \
	ib62x0 ib62x0_second_stage \
	iconnect iconnect_second_stage \
	l-50 \
	nas220 \
	nsa310 \
	nsa310s \
	nsa325 \
	pogo_e02 pogo_e02_second_stage \
	pogoplugv4

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
