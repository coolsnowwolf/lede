#
# Copyright (C) 2019 Sartura Ltd.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-mvebu
PKG_VERSION:=2.2
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE:=$(PKG_NAME)-$(PKG_RELEASE).tar.xz
PKG_SOURCE_URL:=https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
PKG_SOURCE_DATE:=2020-01-21
PKG_SOURCE_VERSION:=3b3d406ebc6f5dae6b5c9170c674183026e7301b
PKG_MIRROR_HASH:=7c63da237c52808e1fe49458d5a4ac804a8eb4c365e26c6b2b709256e8fc041f

PKG_MAINTAINER:=Vladimir Vid <vladimir.vid@sartura.hr>

include $(INCLUDE_DIR)/package.mk

define Package/arm-trusted-firmware-mvebu
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=ARM Trusted Firmware for mvebu devices
endef

define Package/arm-trusted-firmware-mvebu/uDPU
$(call Package/arm-trusted-firmware-mvebu)
  BOARDNAME:=uDPU
  TITLE:=ARM Trusted Firmware for Methode uDPU
  DEPENDS:=+u-boot-uDPU
  UBOOT:=uDPU
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef

A3700_UTILS_NAME:=a3700-utils
A3700_UTILS_RELEASE:=18.12.1
A3700_UTILS_SOURCE=$(A3700_UTILS_NAME)-$(A3700_UTILS_RELEASE).tar.bz2

define Download/a3700-utils
  FILE:=$(A3700_UTILS_SOURCE)
  PROTO:=git
  URL:=https://github.com/MarvellEmbeddedProcessors/A3700-utils-marvell.git
  VERSION:=e6bb176fb9936d9160e53ae09275000dc6440686
  MIRROR_HASH:=c3edf71cbf1fc2e5482d36e36a34c79d682d2c87fdfb909b4dc6327a171e7bd4
  SUBDIR:=$(A3700_UTILS_NAME)
endef

MV_DDR_NAME:=mv-ddr-marvell
MV_DDR_RELEASE:=mainline
MV_DDR_SOURCE:=$(MV_DDR_NAME)-$(MV_DDR_RELEASE).tar.bz2

define Download/mv-ddr-marvell
  FILE:=$(MV_DDR_SOURCE)
  PROTO:=git
  URL:=https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell.git
  VERSION:=a881467ef0f0185e6570dd0483023fde93cbb5f5
  MIRROR_HASH:=19f34817616c583b2b5f2612705857cf2120f6c2132355217899b1403c3fd336
  SUBDIR:=$(MV_DDR_NAME)
endef

LINARO_NAME:=gcc-linaro
LINARO_RELEASE:=6
LINARO_VERSION:=5.0-2018.12-$(HOST_ARCH)_arm-linux-gnueabi
LINARO_SOURCE=$(LINARO_NAME)-$(LINARO_RELEASE).$(LINARO_VERSION).tar.xz

define Download/gcc-linaro
  FILE:=$(LINARO_SOURCE)
  URL:=https://releases.linaro.org/components/toolchain/binaries/latest-$(LINARO_RELEASE)/arm-linux-gnueabi/
  HASH:=2d4a92d6c8b384ae404b2e02c1c412e3ec18f9b714135acf046b2b1b510e9ace
endef

define Build/Prepare
	# Download sources
	$(eval $(call Download,a3700-utils))
	$(eval $(call Download,mv-ddr-marvell))
	$(eval $(call Download,gcc-linaro))

	rm -rf $(PKG_BUILD_DIR)
	$(TAR) -C $(BUILD_DIR) -xf $(DL_DIR)/$(PKG_SOURCE)

	mkdir -p $(STAGING_DIR_IMAGE)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(A3700_UTILS_SOURCE)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(MV_DDR_SOURCE)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(LINARO_SOURCE)
endef

TARGET_CFLAGS = ""

MAKE_VARS= \
	CROSS_COMPILE="$(TARGET_CROSS)"

MAKE_FLAGS = \
	CROSS_CM3=$(STAGING_DIR_IMAGE)/$(LINARO_NAME)-$(LINARO_RELEASE).$(LINARO_VERSION)/bin/arm-linux-gnueabi- \
	BL33=$(STAGING_DIR_IMAGE)/$(UBOOT)-u-boot.bin \
	MV_DDR_PATH=$(STAGING_DIR_IMAGE)/$(MV_DDR_NAME) \
	WTP=$(STAGING_DIR_IMAGE)/$(A3700_UTILS_NAME) \
	CLOCKSPRESET=$(CLOCKSPRESET) \
	PLAT=$(PLAT) \
	all \
	fip

define Build/InstallDev
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/flash-image.bin $(BIN_DIR)/u-boot-$(BOARDNAME)/
endef

$(eval $(call BuildPackage,arm-trusted-firmware-mvebu/uDPU))
