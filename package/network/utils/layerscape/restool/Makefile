#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=restool
PKG_SOURCE_DATE:=2017-12-03
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/qoriq-open-source/restool.git
PKG_SOURCE_VERSION:=90fe5c4054bb9a77adef76c0b16a5af68f3905d3
PKG_MIRROR_HASH:=af016aeaf7f17c668dc18d93c353f687d186c517d7aaee23ed23477c0fc8d42f

include $(INCLUDE_DIR)/package.mk

define Package/restool
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@TARGET_layerscape
  TITLE:=Layerscape DPAA2 dynamical management tool
endef

MAKE_FLAGS += \
	DESTDIR="$(PKG_BUILD_DIR)"/output/ \
	install

define Package/restool/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/output/usr/local/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,restool))
