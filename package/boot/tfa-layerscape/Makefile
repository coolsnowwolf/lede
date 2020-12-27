#
# Copyright 2019 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tfa-layerscape
PKG_VERSION:=LSDK-20.04-update-290520
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/atf
PKG_SOURCE_VERSION:=7d748e6f0ec652ba7c43733dc67a3d0b0217390a
PKG_MIRROR_HASH:=d209c9ad18aac9f18375450b98b8dab00f0382ccb485df14623bf9b72ea1dd9b
PKG_BUILD_DEPENDS:=tfa-layerscape/host

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

HOST_CFLAGS += -Wall -Werror -pedantic -std=c99
define Host/Compile
	$(MAKE) -C \
		$(HOST_BUILD_DIR)/tools/fiptool \
		CFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)"
	$(MAKE) -C \
		$(HOST_BUILD_DIR)/plat/nxp/tools \
		CFLAGS="$(HOST_CFLAGS)"
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tools/fiptool/fiptool $(STAGING_DIR_HOST)/bin/tfa-fiptool
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/plat/nxp/tools/create_pbl $(STAGING_DIR_HOST)/bin/tfa-create-pbl
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/plat/nxp/tools/byte_swap $(STAGING_DIR_HOST)/bin/tfa-byte-swap
endef

define Package/tfa-generic
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_layerscape_armv8_64b +layerscape-rcw +u-boot-fsl_$(subst tfa-,,$(1))
  VARIANT:=$(subst tfa-,,$(1))
endef

define Package/tfa-ls1012a-frdm
  $(Package/tfa-generic)
  TITLE:=NXP LS1012AFRDM Trusted Firmware
  PLAT:=ls1012afrdm
  BOOT_MODE:=qspi
endef

define Package/tfa-ls1012a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS1012ARDB Trusted Firmware
  PLAT:=ls1012ardb
  BOOT_MODE:=qspi
endef

define Package/tfa-ls1012a-frwy-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1012AFRWY Trusted Firmware
  PLAT:=ls1012afrwy
  BOOT_MODE:=qspi
endef

define Package/tfa-ls1043a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS1043ARDB Trusted Firmware
  PLAT:=ls1043ardb
  BOOT_MODE:=nor
endef

define Package/tfa-ls1043a-rdb-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1043ARDB SD Boot Trusted Firmware
  PLAT:=ls1043ardb
  BOOT_MODE:=sd
endef

define Package/tfa-ls1046a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS1046ARDB Trusted Firmware
  PLAT:=ls1046ardb
  BOOT_MODE:=qspi
endef

define Package/tfa-ls1046a-rdb-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1046ARDB SD Boot Trusted Firmware
  PLAT:=ls1046ardb
  BOOT_MODE:=sd
endef

define Package/tfa-ls1088a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS1088ARDB Trusted Firmware
  PLAT:=ls1088ardb
  BOOT_MODE:=qspi
endef

define Package/tfa-ls1088a-rdb-sdboot
  $(Package/tfa-generic)
  TITLE:=NXP LS1088ARDB SD Boot Trusted Firmware
  PLAT:=ls1088ardb
  BOOT_MODE:=sd
endef

define Package/tfa-ls2088a-rdb
  $(Package/tfa-generic)
  TITLE:=NXP LS2088ARDB Trusted Firmware
  PLAT:=ls2088ardb
  BOOT_MODE:=nor
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl2_$(BOOT_MODE).pbl \
		$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-bl2.pbl
	$(CP) $(PKG_BUILD_DIR)/build/$(PLAT)/release/fip.bin \
		$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-fip.bin
endef

define Build/Compile
	$(eval $(Package/tfa-$(BUILD_VARIANT))) \
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE=$(TARGET_CROSS) \
		fip pbl PLAT=$(PLAT) BOOT_MODE=$(BOOT_MODE) \
		RCW=$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-rcw.bin \
		BL33=$(STAGING_DIR_IMAGE)/fsl_$(BUILD_VARIANT)-uboot.bin \
		FIPTOOL=$(STAGING_DIR_HOST)/bin/tfa-fiptool \
		CREATE_PBL=$(STAGING_DIR_HOST)/bin/tfa-create-pbl \
		BYTE_SWAP=$(STAGING_DIR_HOST)/bin/tfa-byte-swap
endef

TFAS := \
  ls1012a-frdm \
  ls1012a-rdb \
  ls1012a-frwy-sdboot \
  ls1043a-rdb \
  ls1043a-rdb-sdboot \
  ls1046a-rdb \
  ls1046a-rdb-sdboot \
  ls1088a-rdb \
  ls1088a-rdb-sdboot \
  ls2088a-rdb

$(eval $(call HostBuild))
$(foreach tfa,$(TFAS), \
  $(eval $(call BuildPackage,tfa-$(tfa))) \
)
