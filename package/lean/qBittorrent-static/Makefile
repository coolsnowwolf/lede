#
# Copyright (C) 2017-2020
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qBittorrent-static
PKG_VERSION:=4.3.7_v1.2.14
PKG_RELEASE=1

STRIP:=true

ifeq ($(ARCH),x86_64)
	PKG_ARCH:=x86_64
endif

ifeq ($(ARCH),arm64)
	PKG_ARCH:=aarch64
endif

ifeq ($(ARCH),aarch64)
	PKG_ARCH:=aarch64
endif

ifeq ($(ARCH),arm)
ifeq ($(BOARD),bcm53xx)
	PKG_ARCH:=armv7
else ifeq ($(BOARD),kirkwood)
	PKG_ARCH:=armv7
else
	PKG_ARCH:=armhf
endif
endif

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=@(arm||aarch64||x86_64)
	SUBMENU:=BitTorrent
	TITLE:=bittorrent client programmed in C++ / Qt
	URL:=https://www.qbittorrent.org/
endef

define Package/$(PKG_NAME)/description
  qBittorrent is a bittorrent client programmed in C++ / Qt that uses
  libtorrent (sometimes called libtorrent-rasterbar) by Arvid Norberg.
  It aims to be a good alternative to all other bittorrent clients out
  there. qBittorrent is fast, stable and provides unicode support as
  well as many features.
endef

define Download/qbittorrent
  URL:=https://github.com/userdocs/qbittorrent-nox-static/releases/download/release-$(PKG_VERSION)
  URL_FILE:=$(PKG_ARCH)-qbittorrent-nox
  FILE:=qbittorrent-nox
  HASH:=skip
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(DL_DIR)/qbittorrent-nox $(1)/usr/bin
endef

$(eval $(call Download,qbittorrent))
$(eval $(call BuildPackage,$(PKG_NAME)))
