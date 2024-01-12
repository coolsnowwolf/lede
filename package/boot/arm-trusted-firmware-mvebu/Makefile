#
# Copyright (C) 2019 Sartura Ltd.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2.4
PKG_RELEASE:=1
PKG_HASH:=bf3eb3617a74cddd7fb0e0eacbfe38c3258ee07d4c8ed730deef7a175cc3d55b

PKG_MAINTAINER:=Vladimir Vid <vladimir.vid@sartura.hr>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  BUILD_TARGET:=mvebu
  BUILD_SUBTARGET:=cortexa53
  TFA_IMAGE:=flash-image.bin uart-images.tgz.bin
  UBOOT:=
  DDR_TOPOLOGY:=
  CLOCKSPRESET:=
endef


define Trusted-Firmware-A/espressobin-512mb
  NAME:=Marvell ESPRESSObin (512MB)
  DEPENDS:=+u-boot-espressobin
  BUILD_DEVICES:=globalscale_espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=0
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef

define Trusted-Firmware-A/espressobin-v3-v5-1gb-1cs
  NAME:=Marvell ESPRESSObin V3-V5 (1GB 1CS)
  DEPENDS:=+u-boot-espressobin
  BUILD_DEVICES:=globalscale_espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=4
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef

define Trusted-Firmware-A/espressobin-v3-v5-1gb-2cs
  NAME:=Marvell ESPRESSObin V3-V5 (1GB, 2CS)
  DEPENDS:=+u-boot-espressobin
  BUILD_DEVICES:=globalscale_espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=2
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef

define Trusted-Firmware-A/espressobin-v3-v5-2gb
  NAME:=Marvell ESPRESSObin V3-V5 (2GB)
  DEPENDS:=+u-boot-espressobin
  BUILD_DEVICES:=globalscale_espressobin
  UBOOT:=espressobin
  DDR_TOPOLOGY:=7
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef

define Trusted-Firmware-A/espressobin-v7-1gb
  NAME:=Marvell ESPRESSObin V7 (1GB)
  DEPENDS:=+u-boot-espressobin
  BUILD_DEVICES:=globalscale_espressobin-v7
  UBOOT:=espressobin
  DDR_TOPOLOGY:=5
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef

define Trusted-Firmware-A/espressobin-v7-2gb
  NAME:=Marvell ESPRESSObin V7 (2GB)
  DEPENDS:=+u-boot-espressobin
  BUILD_DEVICES:=globalscale_espressobin-v7
  UBOOT:=espressobin
  DDR_TOPOLOGY:=6
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef

define Trusted-Firmware-A/udpu
  NAME:=Methode uDPU
  DEPENDS:=+u-boot-uDPU
  BUILD_DEVICES:=methode_udpu
  UBOOT:=uDPU
  DDR_TOPOLOGY:=0
  CLOCKSPRESET:=CPU_1000_DDR_800
  PLAT:=a3700
endef


TFA_TARGETS:= \
	espressobin-512mb \
	espressobin-v3-v5-1gb-1cs \
	espressobin-v3-v5-1gb-2cs \
	espressobin-v3-v5-2gb \
	espressobin-v7-1gb \
	espressobin-v7-2gb \
	udpu

TFA_MAKE_FLAGS += \
		CROSS_CM3=$(STAGING_DIR_IMAGE)/$(LINARO_NAME)-$(LINARO_RELEASE).$(LINARO_VERSION)/bin/arm-linux-gnueabi- \
		BL33=$(STAGING_DIR_IMAGE)/$(UBOOT)-u-boot.bin \
		MV_DDR_PATH=$(STAGING_DIR_IMAGE)/$(MV_DDR_NAME) \
		WTP=$(STAGING_DIR_IMAGE)/$(A3700_UTILS_NAME) \
		DDR_TOPOLOGY=$(DDR_TOPOLOGY) \
		CLOCKSPRESET=$(CLOCKSPRESET) \
		A3700_UTILS_COMMIT_ID=$(A3700_UTILS_RELEASE) \
		MV_DDR_COMMIT_ID=$(MV_DDR_RELEASE) \
		all \
		mrvl_flash

A3700_UTILS_NAME:=a3700-utils
A3700_UTILS_RELEASE:=5598e150
A3700_UTILS_SOURCE=$(A3700_UTILS_NAME)-$(A3700_UTILS_RELEASE).tar.bz2

define Download/a3700-utils
  FILE:=$(A3700_UTILS_SOURCE)
  PROTO:=git
  URL:=https://github.com/MarvellEmbeddedProcessors/A3700-utils-marvell.git
  VERSION:=5598e150fa3a1568256c30223fd2b214d729f26a
  MIRROR_HASH:=4c3a3bed97833d08af4e42995c0c5af6c107f990fd492cd90aa3e79134d2751e
  SUBDIR:=$(A3700_UTILS_NAME)
endef

MV_DDR_NAME:=mv-ddr-marvell
MV_DDR_RELEASE:=6fb99002
MV_DDR_SOURCE:=$(MV_DDR_NAME)-$(MV_DDR_RELEASE).tar.bz2

define Download/mv-ddr-marvell
  FILE:=$(MV_DDR_SOURCE)
  PROTO:=git
  URL:=https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell.git
  VERSION:=6fb99002be5dec9c7f5375b074f53148dbc0739c
  MIRROR_HASH:=6836e5ea47618a7ee2f96a1a6bd8218f003789b877e521fdfcb008f2f6475dd6
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
	$(call PatchDir/Default,$(STAGING_DIR_IMAGE)/$(A3700_UTILS_NAME),./patches-a3700-utils)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(MV_DDR_SOURCE)
	$(call PatchDir/Default,$(STAGING_DIR_IMAGE)/$(MV_DDR_NAME),./patches-mv-ddr-marvell)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(LINARO_SOURCE)
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
