#
# This is free software, licensed under your choice of the following SPDX identifiers:
# Apache-2.0, BSD-2-Clause, CC-BY-4.0, ISC, MIT, X11 or GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=linotify
PKG_VERSION:=0.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=https://codeload.github.com/hoelzro/$(PKG_NAME)/tar.gz/$(PKG_VERSION)?
PKG_HASH:=21d005f16043397a85913a7d14a267716d6384f0228718d5de464bfc0274e338

PKG_MAINTAINER:=Karl Palsson <karlp@etactica.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYRIGHT

include $(INCLUDE_DIR)/package.mk

define Package/linotify
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=inotify binding for lua
  DEPENDS:=+lua
  URL:=https://github.com/hoelzro/linotify
endef

define Package/linotify/description
 This provides a lua binding to the inotify file system change notification system
endef

define Build/Compile
  cd $(PKG_BUILD_DIR) && \
  $(TARGET_CC) -o inotify.so -shared $(FPIC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) linotify.c
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/inotify.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,linotify))
