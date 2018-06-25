#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-mc
PKG_SOURCE_DATE:=2014-07-02
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXP/qoriq-mc-binary.git
PKG_SOURCE_VERSION:=40b0c8b7edfd2d8f989619b12d6352472455c696
PKG_MIRROR_HASH:=b3c8db70de8617e5bc033951b5ad3824f98353303870d6035668075daf93382e

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-mc-ls1088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@TARGET_layerscape
  TITLE:=NXP LS1088ARDB MC firmware
  VARIANT:=ls1088ardb
  CONFIG:=ls1088a/mc_10.4.0_ls1088a_20171101.itb
endef

define Package/layerscape-mc-ls2088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@TARGET_layerscape
  TITLE:=NXP LS2088ARDB MC firmware
  VARIANT:=ls2088ardb
  CONFIG:=ls2088a/mc_10.4.0_ls2088a_20171101.itb
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
