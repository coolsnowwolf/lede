#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-dpl
PKG_SOURCE_DATE:=2017-09-07
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/qoriq-open-source/dpl-examples.git
PKG_SOURCE_VERSION:=a6c83759c0d9c02822eec89e86357a0998ef51d4
PKG_MIRROR_HASH:=d0311591843fbf9faebc7e2f0d53cba47d42ef21c4407c5881f354df6c458ce5

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-dpl-ls1088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@TARGET_layerscape
  TITLE:=NXP LS1088ARDB DPL firmware
  VARIANT:=ls1088ardb
  DPC_CONFIG:=ls1088a/RDB/dpc.0x1D-0x0D.dtb
  DPL_CONFIG:=ls1088a/RDB/dpl-eth.0x1D_0x0D.dtb
endef

define Package/layerscape-dpl-ls2088ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@TARGET_layerscape
  TITLE:=NXP LS2088ARDB DPL firmware
  VARIANT:=ls2088ardb
  DPC_CONFIG:=ls2088a/RDB/dpc.0x2A_0x41.dtb
  DPL_CONFIG:=ls2088a/RDB/dpl-eth.0x2A_0x41.dtb
endef

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
