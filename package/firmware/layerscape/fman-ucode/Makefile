#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fman-ucode
PKG_VERSION:=lsdk-1903
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXP/qoriq-fm-ucode.git
PKG_SOURCE_VERSION:=c275e91392e2adab1ed22f3867b8269ca3c54014
PKG_MIRROR_HASH:=a630f1a86e4a67a0e390312291702560e3f64dd74fe6ccb8f3be4584d00c7f80

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-fman-ls1043ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP LS1043ARDB FMan ucode
  DEPENDS:=@TARGET_layerscape
  CONFIG:=fsl_fman_ucode_ls1043_r1.1_106_4_18.bin
endef

define Package/layerscape-fman-ls1046ardb
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP LS1046ARDB FMan ucode
  DEPENDS:=@TARGET_layerscape
  CONFIG:=fsl_fman_ucode_ls1046_r1.0_106_4_18.bin
endef

define Build/Compile
endef

define Package/layerscape-fman-ls1043ardb/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(CONFIG) $(STAGING_DIR_IMAGE)/ls1043ardb-fman.bin
endef

define Package/layerscape-fman-ls1046ardb/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(CONFIG) $(STAGING_DIR_IMAGE)/ls1046ardb-fman.bin
endef

$(eval $(call BuildPackage,layerscape-fman-ls1043ardb))
$(eval $(call BuildPackage,layerscape-fman-ls1046ardb))
