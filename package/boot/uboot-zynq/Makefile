#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=u-boot
PKG_VERSION:=2016.03
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot

PKG_HASH:=e49337262ecac44dbdeac140f2c6ebd1eba345e0162b0464172e7f05583ed7bb

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

include $(INCLUDE_DIR)/package.mk

define uboot/Default
	TITLE:=
	CONFIG:=
	IMAGE:=
endef

define uboot/zc702
	TITLE:=U-Boot $(PKG_VERSION) for Xilinx ZC702 Dev Board
endef

define uboot/zc706
	TITLE:=U-Boot $(PKG_VERSION) for Xilinx ZC706 Dev Board
endef

define uboot/zed
	TITLE:=U-Boot $(PKG_VERSION) for Avnet Digilent ZedBoard Dev Board
endef

define uboot/zybo
	TITLE:=U-Boot $(PKG_VERSION) for Digilent Zybo Dev Board
endef

UBOOTS := \
	zc702 \
	zed \
	zybo \

define Package/uboot/template
define Package/uboot-zynq-$(1)
	SECTION:=boot
	CATEGORY:=Boot Loaders
	DEPENDS:=@TARGET_zynq
	TITLE:=$(2)
	URL:=http://www.denx.de/wiki/U-Boot
	VARIANT:=$(1)
	MAINTAINER:=Jason Wu <jason.wu.misc@gmail.com>
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
		$(BOARD)_$(UBOOT_CONFIG)_config
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) DTC=$(LINUX_DIR)/scripts/dtc/dtc
endef

define Package/uboot/install/default
	$(INSTALL_DIR) $(BIN_DIR)/uboot-$(BOARD)-$(1)

	$(CP) $(PKG_BUILD_DIR)/u-boot \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/u-boot.elf

	$(CP) $(PKG_BUILD_DIR)/u-boot-dtb.bin \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/u-boot-dtb.bin

	$(CP) $(PKG_BUILD_DIR)/u-boot.dtb \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/u-boot.dtb

	$(CP) $(PKG_BUILD_DIR)/u-boot-dtb.img \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/u-boot-dtb.img

	$(CP) $(PKG_BUILD_DIR)/spl/boot.bin \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/boot.bin

	$(CP) $(PKG_BUILD_DIR)/spl/u-boot-spl \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/u-boot-spl

	$(CP) $(PKG_BUILD_DIR)/spl/u-boot-spl-dtb.bin \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/u-boot-spl-dtb.bin
endef

define Package/uboot/install/template
define Package/uboot-zynq-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-zynq-$(u))) \
)
