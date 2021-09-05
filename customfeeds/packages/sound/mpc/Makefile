#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mpc
PKG_VERSION:=0.33
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.musicpd.org/download/mpc/0
PKG_HASH:=4f40ccbe18f5095437283cfc525a97815e983cbfd3a29e48ff610fa4f1bf1296

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS:=meson/host
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

MESON_ARGS+=-Diconv=disabled -Ddocumentation=disabled

define Package/mpc
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+libmpdclient
  TITLE:=Music Player Daemon Console Client
  URL:=http://www.musicpd.org/
endef

define Package/mpc/description
	MPD is a music player supporting flac, mp3 and ogg files.
	It is typically controlled over a network using one of it's many
	clients including mpc(console), gmpc(gnome), phpmp(php) etc.
	this is MPC
endef

define Package/mpc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mpc $(1)/usr/bin/
	$(INSTALL_BIN) ./files/pls-handler.sh $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mpc))
