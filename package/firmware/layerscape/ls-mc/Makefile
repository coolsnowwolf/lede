#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-mc
PKG_VERSION:=LSDK-21.08
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXP/qoriq-mc-binary.git
PKG_SOURCE_VERSION:=LSDK-21.08
PKG_MIRROR_HASH:=a8b2398f7958bfc3d45b239d33e0ecd27bd46e8f34693d545590a870be22ca2b

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
	$(CP) $(PKG_BUILD_DIR)/ls1088a/mc_ls1088a_10.28.1.itb \
		$(STAGING_DIR_IMAGE)/fsl_ls1088a-rdb-mc.itb
	$(CP) $(PKG_BUILD_DIR)/ls2088a/mc_ls2088a_10.28.1.itb \
		$(STAGING_DIR_IMAGE)/fsl_ls2088a-rdb-mc.itb
	$(CP) $(PKG_BUILD_DIR)/lx216xa/mc_lx2160a_10.28.1.itb \
		$(STAGING_DIR_IMAGE)/fsl_lx2160a-rdb-mc.itb
endef

$(eval $(call BuildPackage,layerscape-mc))
