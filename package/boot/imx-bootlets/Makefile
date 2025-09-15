#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=imx-bootlets
PKG_VERSION:=10.05.02

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://trabant.uid0.hu/openwrt/
PKG_HASH:=09ecd81a64db5166a235932146faf08d0689bfc7ac04ac9fcc3a5bd809fba74a

include $(INCLUDE_DIR)/package.mk

define Package/imx-bootlets
	SECTION:=boot
	CATEGORY:=Boot Loaders
	TITLE:=i.MX23/i.MX28 bootlets
	DEPENDS:=@TARGET_mxs
endef

define Package/imx-bootlets/description
 i.MX23/i.MX28 bootlets (for oLinuxino)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE="$(TARGET_CROSS)"
endef

define Package/imx-bootlets/install
	@echo Copying boot_prep and power_prep into staging - $(STAGING_DIR)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/boot_prep/boot_prep $(STAGING_DIR)/boot_prep
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/linux_prep/output-target/linux_prep $(STAGING_DIR)/linux_prep
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/power_prep/power_prep $(STAGING_DIR)/power_prep
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/linux_prebuilt.db $(STAGING_DIR)/linux_prebuilt.db
endef

$(eval $(call BuildPackage,imx-bootlets))

