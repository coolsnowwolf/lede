#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fman-ucode
PKG_VERSION:=LSDK-20.04
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXP/qoriq-fm-ucode.git
PKG_SOURCE_VERSION:=c275e91392e2adab1ed22f3867b8269ca3c54014
PKG_MIRROR_HASH:=90b619ed501462b92f34f2fabfa09d6aaa5235990891d1c3132821c7d18a39bd

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-fman
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP FMan ucode
  DEPENDS:=@TARGET_layerscape
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/fsl_fman_ucode_ls1043_r1.1_106_4_18.bin \
		$(STAGING_DIR_IMAGE)/fsl_ls1043a-rdb-fman.bin
	$(CP) $(PKG_BUILD_DIR)/fsl_fman_ucode_ls1046_r1.0_106_4_18.bin \
		$(STAGING_DIR_IMAGE)/fsl_ls1046a-rdb-fman.bin
endef

$(eval $(call BuildPackage,layerscape-fman))
