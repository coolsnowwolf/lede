#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-mc
PKG_VERSION:=lsdk-1903
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXP/qoriq-mc-binary.git
PKG_SOURCE_VERSION:=bd64d1824681b009fd83b0a3699e45b117b72695
PKG_MIRROR_HASH:=17ecfd780b8d884b3843d2b8fbcaf29410fdacdc03ab3fe81ae7ade994d7fc0a

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-mc-ls1088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP LS1088ARDB MC firmware
  DEPENDS:=@TARGET_layerscape
  CONFIG:=ls1088a/mc_10.14.3_ls1088a.itb
endef

define Package/layerscape-mc-ls2088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP LS2088ARDB MC firmware
  DEPENDS:=@TARGET_layerscape
  CONFIG:=ls2088a/mc_10.14.3_ls2088a.itb
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
