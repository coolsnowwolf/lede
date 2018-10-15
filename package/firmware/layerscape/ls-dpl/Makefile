#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-dpl
PKG_VERSION:=lsdk-1806
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/mc-utils
PKG_SOURCE_VERSION:=de3c46f16732f5e050851277b91e82d63818ca58
PKG_MIRROR_HASH:=b19bba34abbb83a204ac925a18425463e508c812c43a59eafc89347fc09d5f0a

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/layerscape-dpl-ls1088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP LS1088ARDB DPL firmware
  DEPENDS:=@TARGET_layerscape
  DPC_CONFIG:=config/ls1088a/RDB/dpc.0x1D-0x0D.dtb
  DPL_CONFIG:=config/ls1088a/RDB/dpl-eth.0x1D_0x0D.dtb
endef

define Package/layerscape-dpl-ls2088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP LS2088ARDB DPL firmware
  DEPENDS:=@TARGET_layerscape
  DPC_CONFIG:=config/ls2088a/RDB/dpc.0x2A_0x41.dtb
  DPL_CONFIG:=config/ls2088a/RDB/dpl-eth.0x2A_0x41.dtb
endef

MAKE_PATH:=config
MAKE_VARS+= \
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH)

define Package/layerscape-dpl-ls1088ardb/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(DPL_CONFIG) $(STAGING_DIR_IMAGE)/ls1088ardb-dpl.dtb
	$(CP) $(PKG_BUILD_DIR)/$(DPC_CONFIG) $(STAGING_DIR_IMAGE)/ls1088ardb-dpc.dtb
endef

define Package/layerscape-dpl-ls2088ardb/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(DPL_CONFIG) $(STAGING_DIR_IMAGE)/ls2088ardb-dpl.dtb
	$(CP) $(PKG_BUILD_DIR)/$(DPC_CONFIG) $(STAGING_DIR_IMAGE)/ls2088ardb-dpc.dtb
endef

$(eval $(call BuildPackage,layerscape-dpl-ls1088ardb))
$(eval $(call BuildPackage,layerscape-dpl-ls2088ardb))
