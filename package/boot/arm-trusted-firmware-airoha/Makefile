include $(TOPDIR)/rules.mk

PKG_VERSION:=2.10
PKG_RELEASE:=3
PKG_HASH:=88215a62291b9ba87da8e50b077741103cdc08fb6c9e1ebd34dfaace746d3201

PKG_MAINTAINER:=Christian Marangi <ansuelsmth@gmail.com>

PKG_BUILD_DEPENDS:=arm-trusted-firmware-tools/host arm-trusted-firmware-airoha/host

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/trusted-firmware-a-airoha-$(PKG_VERSION)
HOST_PATCH_DIR:=$(CURDIR)/host-patches

include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  BUILD_TARGET:=airoha
  PLAT:=en7523
  HIDDEN:=y
endef

define Trusted-Firmware-A/bl2/Default
  NAME:=Airoha an7581 bl2
  ARCH:=aarch32
  ATF_STAGE:=bl2
endef

define Trusted-Firmware-A/an7581-bl2
  $(call Trusted-Firmware-A/bl2/Default)
  BUILD_SUBTARGET:=an7581
  SOC:=EN7581
  BUILD_DEVICES:=airoha_an7581-evb airoha_an7581-evb-emmc-eagle airoha_an7581-evb-emmc-kite
endef

define Trusted-Firmware-A/an7583-bl2
  $(call Trusted-Firmware-A/bl2/Default)
  BUILD_SUBTARGET:=an7583
  SOC:=AN7583
  BUILD_DEVICES:=airoha_an7583-evb airoha_an7583-evb-emmc
endef

define Trusted-Firmware-A/bl31/Default
  NAME:=Airoha an7581 bl31
  ARCH:=aarch64
  ATF_STAGE:=bl31
endef

define Trusted-Firmware-A/an7581-bl31
  $(call Trusted-Firmware-A/bl31/Default)
  BUILD_SUBTARGET:=an7581
  SOC:=EN7581
  BUILD_DEVICES:=airoha_an7581-evb airoha_an7581-evb-emmc-eagle airoha_an7581-evb-emmc-kite
endef

define Trusted-Firmware-A/an7583-bl31
  $(call Trusted-Firmware-A/bl31/Default)
  BUILD_SUBTARGET:=an7583
  SOC:=AN7583
  BUILD_DEVICES:=airoha_an7583-evb airoha_an7583-evb-emmc
endef

TFA_TARGETS:= \
	an7581-bl2 an7581-bl31 \
	an7583-bl2 an7583-bl31

TFA_MAKE_FLAGS= \
	ARM32TOOLCHAIN_BASE=$(BL2_CROSS_COMPILE) \
	MBEDTLS_DIR=$(PKG_BUILD_DIR)/$(MBEDTLS_SOURCE) \
	ARCH=$(ARCH) CONFIG_ECNT=1  \
	TCSUPPORT_OPENWRT=1 TCSUPPORT_ATF_UNOPEN=0 \
	TCSUPPORT_CPU_$(SOC)=1 TCSUPPORT_CPU_EN7523=1 \
	TCSUPPORT_CPU_ARMV8=1 TCSUPPORT_UBOOT_64BIT=1 \
	TCSUPPORT_EMMC=1 TCSUPPORT_UBOOT=1 TCSUPPORT_BL2_OPTIMIZATION=1 \
	TOOLS_DIR=$(STAGING_DIR_HOST)/bin

BL2_GCC_NAME:=arm-gnu-toolchain
BL2_GCC_RELEASE:=15.3.rel1
BL2_GCC_VERSION:=$(HOST_ARCH)-arm-none-eabi
ifeq ($(HOST_OS),Darwin)
  ifneq ($(filter arm64 aarch64,$(HOST_ARCH)),)
    BL2_GCC_VERSION:=darwin-arm64-arm-none-eabi
  else
    # Arm no longer publishes an Intel macOS bundle for this release.
    # Use the native Homebrew cross compiler on Intel Macs.
    BL2_GCC_EXTERNAL:=1
  endif
endif
BL2_GCC_SOURCE=$(BL2_GCC_NAME)-$(BL2_GCC_RELEASE)-$(BL2_GCC_VERSION)
BL2_CROSS_COMPILE=$(if $(BL2_GCC_EXTERNAL),arm-none-eabi-,$(PKG_BUILD_DIR)/$(BL2_GCC_SOURCE)/bin/arm-none-eabi-)

define Download/bl2-toolchain
  FILE:=$(BL2_GCC_SOURCE).tar.xz
  URL:=https://gitlab.arm.com/api/v4/projects/tooling%2Fgnu-toolchains-for-arm/packages/generic/gnu-toolchain/$(BL2_GCC_RELEASE)/
ifeq ($(HOST_OS),Darwin)
  HASH:=376808a59ca209c1413236f1c6a509e33da4b29857ab28642b9927cf3048af55
else ifeq ($(HOST_ARCH),aarch64)
  HASH:=06979e0c8171de58e5dc2a2b2019330a290f30930f27728af98a83e1a7369b3a
else
  HASH:=563bebb2b97d53382b956d6ee1fe61e2cae26699901417234a37df505ef9b5fa
endif
endef

define Download/atf-airoha
  SOURCE_DATE:=2026-09-30
  SOURCE_VERSION:=705478c3d84fb1cba4c2de1bb99c43098b785d61
  URL:=https://github.com/Ansuel/atf-airoha.git
  MIRROR_HASH:=24d08b8d47b7c9fa83cda20bd80b38f5872dc48f8ec770cb82582e7fa0c5b75c
  PROTO:=git
  FILE:=atf-airoha-$$$$(subst -,.,$$$$(SOURCE_DATE))~$$$$(call version_abbrev,$$$$(SOURCE_VERSION)).tar.zst
  SUBDIR:=atf-airoha
endef

define Download/atf-airoha-host
  $(call Download/atf-airoha)
  FILE:=atf-airoha-host-$$$$(subst -,.,$$$$(SOURCE_DATE))~$$$$(call version_abbrev,$$$$(SOURCE_VERSION)).tar.zst
endef

MBEDTLS_NAME:=mbedtls
MBEDTLS_RELEASE:=3.4.1
MBEDTLS_SOURCE:=$(MBEDTLS_NAME)-$(MBEDTLS_RELEASE)

define Download/mbedtls
  FILE:=$(MBEDTLS_SOURCE).tar.zst
  PROTO:=git
  URL:=https://github.com/Mbed-TLS/mbedtls.git
  SOURCE_VERSION:=72718dd87e087215ce9155a826ee5a66cfbe9631
  MIRROR_HASH:=482e699edb915f8530c99f680ac69783036d96d461eebc95382d19a6064e906b
  SUBDIR:=$(MBEDTLS_SOURCE)
endef

define Host/Prepare
	$(eval $(call Download,atf-airoha-host))
	$(TAR) -C $(HOST_BUILD_DIR) --strip-components=1 -xf $(DL_DIR)/$(FILE)
	$(Host/Patch)
endef

HOST_CFLAGS += \
	-DFLASH_TABLE_OPEN \
	-DTCSUPPORT_BL2_OPTIMIZATION \
	-I$(HOST_BUILD_DIR)/plat/ecnt/en7523/include
define Host/Compile
	$(HOSTCC) $(HOST_CFLAGS) -o $(HOST_BUILD_DIR)/spi_nand_flash_table \
		$(HOST_BUILD_DIR)/plat/ecnt/common/drivers/flash/spi_nand_flash_table.c
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/spi_nand_flash_table $(STAGING_DIR_HOST)/bin/airoha-atf-gen-flash-table
endef

define Build/Prepare
	$(if $(filter bl2,$(ATF_STAGE)),
		$(if $(BL2_GCC_EXTERNAL),
			command -v $(BL2_CROSS_COMPILE)gcc >/dev/null || { \
				echo "Airoha BL2 on Intel macOS requires: brew install arm-none-eabi-gcc" >&2; exit 1; },
			$(eval $(call Download,bl2-toolchain))
			$(TAR) -C $(PKG_BUILD_DIR) -xf $(DL_DIR)/$(FILE)
		)
	)

	$(call Build/Prepare/Default,)

	$(eval $(call Download,mbedtls))
	$(TAR) -C $(PKG_BUILD_DIR) -xf $(DL_DIR)/$(FILE)

	$(eval $(call Download,atf-airoha))
	$(TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xf $(DL_DIR)/$(FILE)
endef

define Build/Compile/bl2
	$(Build/Compile/Trusted-Firmware-A) IMAGE_BL21=1 bl2
	$(Build/Compile/Trusted-Firmware-A) clean
	$(Build/Compile/Trusted-Firmware-A) IMAGE_BL22=1 bl2
	$(Build/Compile/Trusted-Firmware-A) clean
	$(Build/Compile/Trusted-Firmware-A) IMAGE_BL23=1 bl2
endef

define Build/Compile/bl31
	$(Build/Compile/Trusted-Firmware-A) bl31
endef

define Build/Compile
	$(call Build/Compile/$(ATF_STAGE))
endef

define Package/trusted-firmware-a/bl2/install
	dd if=/dev/null of=$(PKG_BUILD_DIR)/bl21.bin bs=1 count=0 seek=14336

	(cd $(PKG_BUILD_DIR); $(STAGING_DIR_HOST)/bin/airoha-atf-gen-flash-table)
	$(STAGING_DIR_HOST)/bin/lzma e \
		$(PKG_BUILD_DIR)/flash_table.bin \
		$(PKG_BUILD_DIR)/flash_table.lzma

	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	./scripts/airoha_pack_bl2.sh \
		$(PKG_BUILD_DIR)/bl21.bin \
		$(PKG_BUILD_DIR)/bl22.lzma \
		$(PKG_BUILD_DIR)/bl23.lzma \
		$(PKG_BUILD_DIR)/flash_table.lzma \
		$(STAGING_DIR_IMAGE)/$(BUILD_SUBTARGET)-bl2.bin
endef

define Package/trusted-firmware-a/bl31/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(STAGING_DIR_HOST)/bin/lzma e \
		$(PKG_BUILD_DIR)/build/$(PLAT)/release/bl31.bin \
		$(STAGING_DIR_IMAGE)/$(BUILD_SUBTARGET)-bl31.lzma
endef

define Package/trusted-firmware-a/install
	$(call Package/trusted-firmware-a/$(ATF_STAGE)/install)
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage/Trusted-Firmware-A))
