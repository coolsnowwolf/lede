#
# Copyright 2019 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tfa-layerscape
PKG_VERSION:=LSDK-20.04
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/atf
PKG_SOURCE_VERSION:=4a82c939a0211196e2b80a495f966383803753bb
PKG_MIRROR_HASH:=31c264551ce800b646d7dcc5baa62cfbb4c8fa3299a93931c7ef71dc3e88380d
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

define Package/tfa-layerscape/ls1012afrdm
  TITLE:=NXP LS1012AFRDM Trusted Firmware
  PLAT:=ls1012afrdm
  BOOT_MODE:=qspi
  BIN_BL2:=build/ls1012afrdm/release/bl2_qspi.pbl
  BIN_FIP:=build/ls1012afrdm/release/fip.bin
endef

define Package/tfa-layerscape/ls1012ardb
  TITLE:=NXP LS1012ARDB Trusted Firmware
  PLAT:=ls1012ardb
  BOOT_MODE:=qspi
  BIN_BL2:=build/ls1012ardb/release/bl2_qspi.pbl
  BIN_FIP:=build/ls1012ardb/release/fip.bin
endef

define Package/tfa-layerscape/ls1012afrwy
  TITLE:=NXP LS1012AFRWY Trusted Firmware
  PLAT:=ls1012afrwy
  BOOT_MODE:=qspi
  BIN_BL2:=build/ls1012afrwy/release/bl2_qspi.pbl
  BIN_FIP:=build/ls1012afrwy/release/fip.bin
endef

define Package/tfa-layerscape/ls1043ardb
  TITLE:=NXP LS1043ARDB Trusted Firmware
  PLAT:=ls1043ardb
  BOOT_MODE:=nor
  BIN_BL2:=build/ls1043ardb/release/bl2_nor.pbl
  BIN_FIP:=build/ls1043ardb/release/fip.bin
endef

define Package/tfa-layerscape/ls1043ardb-sdboot
  TITLE:=NXP LS1043ARDB SD Boot Trusted Firmware
  PLAT:=ls1043ardb
  BOOT_MODE:=sd
  BIN_BL2:=build/ls1043ardb/release/bl2_sd.pbl
  BIN_FIP:=build/ls1043ardb/release/fip.bin
endef

define Package/tfa-layerscape/ls1046ardb
  TITLE:=NXP LS1046ARDB Trusted Firmware
  PLAT:=ls1046ardb
  BOOT_MODE:=qspi
  BIN_BL2:=build/ls1046ardb/release/bl2_qspi.pbl
  BIN_FIP:=build/ls1046ardb/release/fip.bin
endef

define Package/tfa-layerscape/ls1046ardb-sdboot
  TITLE:=NXP LS1046ARDB SD Boot Trusted Firmware
  PLAT:=ls1046ardb
  BOOT_MODE:=sd
  BIN_BL2:=build/ls1046ardb/release/bl2_sd.pbl
  BIN_FIP:=build/ls1046ardb/release/fip.bin
endef

define Package/tfa-layerscape/ls1088ardb
  TITLE:=NXP LS1088ARDB Trusted Firmware
  PLAT:=ls1088ardb
  BOOT_MODE:=qspi
  BIN_BL2:=build/ls1088ardb/release/bl2_qspi.pbl
  BIN_FIP:=build/ls1088ardb/release/fip.bin
endef

define Package/tfa-layerscape/ls1088ardb-sdboot
  TITLE:=NXP LS1088ARDB SD Boot Trusted Firmware
  PLAT:=ls1088ardb
  BOOT_MODE:=sd
  BIN_BL2:=build/ls1088ardb/release/bl2_sd.pbl
  BIN_FIP:=build/ls1088ardb/release/fip.bin
endef

define Package/tfa-layerscape/ls2088ardb
  TITLE:=NXP LS2088ARDB Trusted Firmware
  PLAT:=ls2088ardb
  BOOT_MODE:=nor
  BIN_BL2:=build/ls2088ardb/release/bl2_nor.pbl
  BIN_FIP:=build/ls2088ardb/release/fip.bin
endef

define Package/tfa-layerscape/Config
  define Package/tfa-$(1)
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=$(2)
    DEPENDS:=@TARGET_layerscape_armv8_64b +layerscape-rcw +u-boot-$(1)
    VARIANT:=$(1)
    BIN_BL2:=$(3)
    BIN_FIP:=$(4)
  endef
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(BIN_BL2) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl2.pbl
	$(CP) $(PKG_BUILD_DIR)/$(BIN_FIP) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-fip.bin
endef

define Build/Compile
	$(eval $(Package/tfa-layerscape/$(BUILD_VARIANT))) \
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE=$(TARGET_CROSS) \
		fip pbl PLAT=$(PLAT) BOOT_MODE=$(BOOT_MODE) \
		RCW=$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-rcw.bin \
		BL33=$(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot.bin \
		FIPTOOL=$(STAGING_DIR_HOST)/bin/tfa-fiptool \
		CREATE_PBL=$(STAGING_DIR_HOST)/bin/tfa-create-pbl \
		BYTE_SWAP=$(STAGING_DIR_HOST)/bin/tfa-byte-swap
endef

TFAS := \
  ls1012afrdm \
  ls1012ardb \
  ls1012afrwy \
  ls1043ardb \
  ls1043ardb-sdboot \
  ls1046ardb \
  ls1046ardb-sdboot \
  ls1088ardb \
  ls1088ardb-sdboot \
  ls2088ardb

$(eval $(call HostBuild))
$(foreach tfa,$(TFAS), \
  $(eval $(Package/tfa-layerscape/$(tfa))) \
  $(eval $(call Package/tfa-layerscape/Config,$(tfa),$(TITLE),$(BIN_BL2),$(BIN_FIP))) \
  $(eval $(call BuildPackage,tfa-$(tfa))) \
)
