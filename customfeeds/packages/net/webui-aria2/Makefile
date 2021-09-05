#
# Copyright (C) 2016-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=webui-aria2
PKG_SOURCE_DATE:=2018-12-09
PKG_SOURCE_VERSION:=fb9d758d5cdc2be0867ee9502c44fd17560f5d24
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_DATE).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ziahamza/webui-aria2/tar.gz/$(PKG_SOURCE_VERSION)?
PKG_HASH:=4cddcbd64a7b20baf0c2c31cb3c07d0fa01576a12c4e6feefd0840acd3f65919
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_VERSION)

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Hsing-Wang Liao <kuoruan@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/webui-aria2
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Download Manager
  DEPENDS:=
  TITLE:=An interface to interact with aria2
  URL:=https://github.com/ziahamza/webui-aria2
  PKGARCH:=all
endef

define Package/webui-aria2/description
	The aim for this project is to create the worlds best and hottest interface to interact with aria2. Very simple to use.
endef

define Build/Compile
endef

define Package/webui-aria2/install
	$(INSTALL_DIR) $(1)/www/webui-aria2 

	$(CP) \
		$(PKG_BUILD_DIR)/docs/* \
		$(PKG_BUILD_DIR)/{LICENSE,favicon.ico} \
		$(1)/www/webui-aria2
endef

$(eval $(call BuildPackage,webui-aria2))
