#
# Copyright (C) 2019 Sartura Ltd.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-mvebu
PKG_VERSION:=2.3
PKG_RELEASE:=1

PKG_SOURCE:=trusted-firmware-a-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/snapshot
PKG_HASH:=37f917922bcef181164908c470a2f941006791c0113d738c498d39d95d543b21
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/trusted-firmware-a-$(PKG_VERSION)

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=docs/license.rst

PKG_MAINTAINER:=Vladimir Vid <vladimir.vid@sartura.hr>

include $(INCLUDE_DIR)/package.mk

define Package/arm-trusted-firmware-mvebu
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_mvebu_cortexa53
  VARIANT:=$(subst arm-trusted-firmware-mvebu-,,$(1))
endef

define Package/arm-trusted-firmware-mvebu-espressobin-512mb
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin (512MB)
  DEPENDS:=+u-boot-espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=0
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v3-v5-1gb-1cs
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V3-V5 (1GB 1CS)
  DEPENDS:=+u-boot-espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=2
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v3-v5-1gb-1cs-emmc
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V3-V5 (1GB 1CS, eMMC)
  DEPENDS:=+u-boot-espressobin-emmc
  UBOOT:=espressobin-emmc
  DDR_TOPOLOGY:=4
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v3-v5-1gb-2cs
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V3-V5 (1GB, 2CS)
  DEPENDS:=+u-boot-espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=2
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v3-v5-1gb-2cs-emmc
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V3-V5 (1GB 2CS, eMMC)
  DEPENDS:=+u-boot-espressobin-emmc
  UBOOT:=espressobin-emmc
  DDR_TOPOLOGY:=2
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v3-v5-2gb
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V3-V5 (2GB)
  DEPENDS:=+u-boot-espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=7
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v3-v5-2gb-emmc
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V3-V5 (2GB, eMMC)
  DEPENDS:=+u-boot-espressobin-emmc
  UBOOT:=espressobin-emmc
  DDR_TOPOLOGY:=7
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v7-1gb
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V7 (1GB)
  DEPENDS:=+u-boot-espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=5
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v7-1gb-emmc
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V7 (1GB, eMMC)
  DEPENDS:=+u-boot-espressobin
  UBOOT:=espressobin-emmc
  DDR_TOPOLOGY:=5
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v7-2gb
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V7 (2GB)
  DEPENDS:=+u-boot-espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=6
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-espressobin-v7-2gb-emmc
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Marvell ESPRESSObin V7 (2GB, eMMC)
  DEPENDS:=+u-boot-espressobin
  UBOOT:=espressobin-emmc
  DDR_TOPOLOGY:=6
  CLOCKSPRESET:=CPU_800_DDR_800
  PLAT:=a3700
endef

define Package/arm-trusted-firmware-mvebu-udpu
  $(Package/arm-trusted-firmware-mvebu)
  TITLE:=ARM Trusted Firmware for Methode uDPU
  DEPENDS:=+u-boot-uDPU
  UBOOT:=uDPU
  DDR_TOPOLOGY:=0
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef

A3700_UTILS_NAME:=a3700-utils
A3700_UTILS_RELEASE:=09679790
A3700_UTILS_SOURCE=$(A3700_UTILS_NAME)-$(A3700_UTILS_RELEASE).tar.bz2

define Download/a3700-utils
  FILE:=$(A3700_UTILS_SOURCE)
  PROTO:=git
  URL:=https://github.com/MarvellEmbeddedProcessors/A3700-utils-marvell.git
  VERSION:=096797908ddd69a679fd55595c41fc02809829a9
  MIRROR_HASH:=9276d63d41b2e221c2fd5cceee988ca26eff5005747787f084868b3940173b7a
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

	$(call Build/Prepare/Default,)

	mkdir -p $(STAGING_DIR_IMAGE)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(A3700_UTILS_SOURCE)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(MV_DDR_SOURCE)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(LINARO_SOURCE)
endef

export GCC_HONOUR_COPTS=s

TARGET_CFLAGS = ""

define Build/Compile
	$(eval $(Package/arm-trusted-firmware-mvebu-$(BUILD_VARIANT))) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		OPENSSL_DIR=$(STAGING_DIR_HOST) \
		CROSS_CM3=$(STAGING_DIR_IMAGE)/$(LINARO_NAME)-$(LINARO_RELEASE).$(LINARO_VERSION)/bin/arm-linux-gnueabi- \
		BL33=$(STAGING_DIR_IMAGE)/$(UBOOT)-u-boot.bin \
		MV_DDR_PATH=$(STAGING_DIR_IMAGE)/$(MV_DDR_NAME) \
		WTP=$(STAGING_DIR_IMAGE)/$(A3700_UTILS_NAME) \
		DDR_TOPOLOGY=$(DDR_TOPOLOGY) \
		CLOCKSPRESET=$(CLOCKSPRESET) \
		PLAT=$(PLAT) \
		all \
		fip
endef

define Build/InstallDev
	$(INSTALL_DIR) $(BIN_DIR)/flash-image-$(BUILD_VARIANT)
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/flash-image.bin $(BIN_DIR)/flash-image-$(BUILD_VARIANT)/
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/uart-images.tgz $(BIN_DIR)/flash-image-$(BUILD_VARIANT)/
endef

$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-512mb))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v3-v5-1gb-1cs))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v3-v5-1gb-1cs-emmc))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v3-v5-1gb-2cs))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v3-v5-1gb-2cs-emmc))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v3-v5-2gb))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v3-v5-2gb-emmc))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v7-1gb))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v7-1gb-emmc))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v7-2gb))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-espressobin-v7-2gb-emmc))
$(eval $(call BuildPackage,arm-trusted-firmware-mvebu-udpu))
