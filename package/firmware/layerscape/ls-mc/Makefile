#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-mc
PKG_VERSION:=LSDK-20.04
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXP/qoriq-mc-binary.git
PKG_SOURCE_VERSION:=f73683596a7b72124d67b62e64f3dc2bb36b9321
PKG_MIRROR_HASH:=1cba30c2a6814763c3e155c1cc5fa21998bb6ad5814fcb09e99f98bf36f65d9e

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/layerscape-mc
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP MC firmware
  DEPENDS:=@TARGET_layerscape
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/ls1088a/mc_10.20.4_ls1088a.itb \
		$(STAGING_DIR_IMAGE)/fsl_ls1088a-rdb-mc.itb
	$(CP) $(PKG_BUILD_DIR)/ls2088a/mc_10.20.4_ls2088a.itb \
		$(STAGING_DIR_IMAGE)/fsl_ls2088a-rdb-mc.itb
	$(CP) $(PKG_BUILD_DIR)/lx2160a/mc_10.20.4_lx2160a.itb \
		$(STAGING_DIR_IMAGE)/fsl_lx2160a-rdb-mc.itb
endef

$(eval $(call BuildPackage,layerscape-mc))
