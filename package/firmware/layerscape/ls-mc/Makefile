#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-mc
PKG_VERSION:=lsdk-1806
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXP/qoriq-mc-binary.git
PKG_SOURCE_VERSION:=9c1d73016b47f8654d48f0ccff776e5e4588d822
PKG_MIRROR_HASH:=b50e98d7de3c1754fc1d5aed2cb5a10c5d3535bc609407fafbab8ecce09661fa

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-mc-ls1088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP LS1088ARDB MC firmware
  DEPENDS:=@TARGET_layerscape
  CONFIG:=ls1088a/mc_10.8.0_ls1088a_20180515.itb
endef

define Package/layerscape-mc-ls2088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP LS2088ARDB MC firmware
  DEPENDS:=@TARGET_layerscape
  CONFIG:=ls2088a/mc_10.8.0_ls2088a_20180515.itb
endef

define Build/Compile
endef

define Package/layerscape-mc-ls1088ardb/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(CONFIG) $(STAGING_DIR_IMAGE)/ls1088ardb-mc.itb
endef

define Package/layerscape-mc-ls2088ardb/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(CONFIG) $(STAGING_DIR_IMAGE)/ls2088ardb-mc.itb
endef

$(eval $(call BuildPackage,layerscape-mc-ls1088ardb))
$(eval $(call BuildPackage,layerscape-mc-ls2088ardb))
