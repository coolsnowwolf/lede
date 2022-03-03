#
# Copyright (C) 2019 Sartura Ltd.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2.5
PKG_RELEASE:=$(AUTORELEASE)
PKG_HASH:=ad8a2ffcbcd12d919723da07630fc0840c3c2fba7656d1462e45488e42995d7c

PKG_MAINTAINER:=Vladimir Vid <vladimir.vid@sartura.hr>

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
		CROSS_CM3=$(STAGING_DIR_IMAGE)/$(CM3_GCC_NAME)-$(CM3_GCC_RELEASE)-$(CM3_GCC_VERSION)/bin/arm-none-eabi- \
		BL33=$(STAGING_DIR_IMAGE)/$(UBOOT)-u-boot.bin \
		MV_DDR_PATH=$(STAGING_DIR_IMAGE)/$(MV_DDR_NAME) \
		WTP=$(STAGING_DIR_IMAGE)/$(A3700_UTILS_NAME) \
		WTMI_IMG=$(STAGING_DIR_IMAGE)/$(MOX_BB_NAME)-$(MOX_BB_RELEASE)/wtmi_app.bin \
		CRYPTOPP_PATH=$(STAGING_DIR_IMAGE)/$(CRYPTOPP_NAME) \
		USE_COHERENT_MEM=0 \
		FIP_ALIGN=0x100 \
		DDR_TOPOLOGY=$(DDR_TOPOLOGY) \
		CLOCKSPRESET=$(CLOCKSPRESET) \
		A3700_UTILS_COMMIT_ID=$(A3700_UTILS_RELEASE) \
		MV_DDR_COMMIT_ID=$(MV_DDR_RELEASE) \
		all \
		mrvl_flash \
		mrvl_uart

A3700_UTILS_NAME:=a3700-utils
A3700_UTILS_RELEASE:=97f01f5f
A3700_UTILS_SOURCE=$(A3700_UTILS_NAME)-$(A3700_UTILS_RELEASE).tar.bz2

define Download/a3700-utils
  FILE:=$(A3700_UTILS_SOURCE)
  PROTO:=git
  URL:=https://github.com/MarvellEmbeddedProcessors/A3700-utils-marvell.git
  VERSION:=97f01f5feaf9ef6168e2a2096abaf56371939e58
  MIRROR_HASH:=1e391c4dafb7b3363a17282e229d056a95575c0f4fb2f71e21db8044668aea78
  SUBDIR:=$(A3700_UTILS_NAME)
endef

CRYPTOPP_NAME:=cryptopp
CRYPTOPP_RELEASE:=f210224
CRYPTOPP_SOURCE=$(CRYPTOPP_NAME)-$(CRYPTOPP_RELEASE).tar.bz2

define Download/cryptopp
  FILE:=$(CRYPTOPP_SOURCE)
  PROTO:=git
  URL:=https://github.com/weidai11/cryptopp.git
  VERSION:=f2102243e6fdd48c0b2a393a0993cca228f20573
  MIRROR_HASH:=74ec9e48ee04b9f2d9a1d8c4f2392ed0ab52780d7af0f70405d7bbb23d1504fa
  SUBDIR:=$(CRYPTOPP_NAME)
endef

MV_DDR_NAME:=mv-ddr-marvell
MV_DDR_RELEASE:=efcad0e2
MV_DDR_SOURCE:=$(MV_DDR_NAME)-$(MV_DDR_RELEASE).tar.bz2

define Download/mv-ddr-marvell
  FILE:=$(MV_DDR_SOURCE)
  PROTO:=git
  URL:=https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell.git
  VERSION:=efcad0e2fae66a8b6f84a4dd2326f5add67569d5
  MIRROR_HASH:=99363f928ee1239fd42d651b495d163a60cdab00c24770a3c5e192efa7169d62
  SUBDIR:=$(MV_DDR_NAME)
endef

MOX_BB_NAME:=mox-boot-builder
MOX_BB_RELEASE:=v2021.09.07
MOX_BB_SOURCE:=$(MOX_BB_NAME)-$(MOX_BB_RELEASE).tar.bz2

define Download/mox-boot-builder
  FILE:=$(MOX_BB_SOURCE)
  URL:=https://gitlab.nic.cz/turris/mox-boot-builder/-/archive/$(MOX_BB_RELEASE)
  HASH:=fd5fe276a3b0dee3177d61c017907a8eb23cd2169478fa78e9a3a836cfe3a4a8
endef

CM3_GCC_NAME:=gcc-arm
CM3_GCC_RELEASE:=10.2-2020.11
CM3_GCC_VERSION:=$(HOST_ARCH)-arm-none-eabi
CM3_GCC_SOURCE=$(CM3_GCC_NAME)-$(CM3_GCC_RELEASE)-$(CM3_GCC_VERSION).tar.xz

define Download/cm3-gcc
  FILE:=$(CM3_GCC_SOURCE)
  URL:=https://developer.arm.com/-/media/Files/downloads/gnu-a/$(CM3_GCC_RELEASE)/binrel
ifeq ($(HOST_ARCH),aarch64)
  HASH:=1a42eecafa03dc6f32b8ae49ffcd15114dc818efbd72292fa6bab58233940bb9
else
  HASH:=bf7ee185936d22d787b80c8da573f72ead5675695331fb8b590f0133ef1f6bb9
endif
endef

define Build/Clean
	rm -rf \
		$(STAGING_DIR_IMAGE)/$(CRYPTOPP_NAME) \
		$(STAGING_DIR_IMAGE)/$(A3700_UTILS_NAME) \
		$(STAGING_DIR_IMAGE)/$(MV_DDR_NAME) \
		$(STAGING_DIR_IMAGE)/$(MOX_BB_NAME)-$(MOX_BB_RELEASE) \
		$(STAGING_DIR_IMAGE)/$(CM3_GCC_NAME)-$(CM3_GCC_RELEASE)-$(CM3_GCC_VERSION)
endef

define Build/Prepare
	# Download sources
	$(eval $(call Download,a3700-utils))
	$(eval $(call Download,mv-ddr-marvell))
	$(eval $(call Download,mox-boot-builder))
	$(eval $(call Download,cryptopp))
	$(eval $(call Download,cm3-gcc))

	$(call Build/Prepare/Default,)

	mkdir -p $(STAGING_DIR_IMAGE)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(CRYPTOPP_SOURCE)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(A3700_UTILS_SOURCE)
	echo "master" > $(STAGING_DIR_IMAGE)/$(A3700_UTILS_NAME)/branch.txt
	$(call PatchDir/Default,$(STAGING_DIR_IMAGE)/$(A3700_UTILS_NAME),./patches-a3700-utils)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(MV_DDR_SOURCE)
	echo "master" > $(STAGING_DIR_IMAGE)/$(MV_DDR_NAME)/branch.txt
	$(call PatchDir/Default,$(STAGING_DIR_IMAGE)/$(MV_DDR_NAME),./patches-mv-ddr-marvell)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(MOX_BB_SOURCE)
	$(call PatchDir/Default,$(STAGING_DIR_IMAGE)/$(MOX_BB_NAME)-$(MOX_BB_RELEASE),./patches-mox-boot-builder)
	$(TAR) -C $(STAGING_DIR_IMAGE) -xf $(DL_DIR)/$(CM3_GCC_SOURCE)
endef

define Build/Compile
	+$(MAKE) \
		CROSS_CM3=$(STAGING_DIR_IMAGE)/$(CM3_GCC_NAME)-$(CM3_GCC_RELEASE)-$(CM3_GCC_VERSION)/bin/arm-none-eabi- \
		WTMI_VERSION=$(MOX_BB_RELEASE) \
		CRYPTOPP_PATH=$PWD/cryptopp/ \
		-C $(STAGING_DIR_IMAGE)/$(MOX_BB_NAME)-$(MOX_BB_RELEASE) \
		wtmi_app.bin
	$(call Build/Compile/Default)
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
