#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=oggfwd
PKG_REV:=506088e5066abd086efc951670906187c120e9a3
PKG_VERSION:=20150412
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MIRROR_HASH:=7d38d6cb57529d832d87a49d23d3ce24ba088110e3721580139860486c15494c
PKG_SOURCE_URL:=https://r-w-x.org/oggfwd.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Nicolas Thill <nico@openwrt.org>

include $(INCLUDE_DIR)/package.mk

define Package/oggfwd
  SECTION:=multimedia
  CATEGORY:=Multimedia
  SUBMENU:=Streaming
  TITLE:=Pipe media into icecast
  URL:=http://www.v2v.cc/~j/oggfwd
  DEPENDS:=+libpthread +libshout-full
endef

define Package/oggfwd/description
 A basic icecast client that is very simple but also very useful for video
 streaming.
endef

MAKE_FLAGS += \
	INCLUDEFLAGS="$(TARGET_LDFLAGS)" \
	LINKFLAGS="$(TARGET_LDFLAGS) -lshout" \

define Package/oggfwd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oggfwd $(1)/usr/bin/
endef

$(eval $(call BuildPackage,oggfwd))
