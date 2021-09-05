#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=forked-daapd
PKG_VERSION:=27.4
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/ejurgensen/$(PKG_NAME)/releases/download/$(PKG_VERSION)/
PKG_HASH:=00f71c687df268a3c4db77cecf37972e76832a99e34f09750d07a92934a0bfa8

PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_INSTALL:=1

PKG_MAINTAINER:=Espen Jürgensen <espenjurgensen+openwrt@gmail.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/forked-daapd
SECTION:=sound
CATEGORY:=Sound
TITLE:=iTunes (DAAP) server for Apple Remote and AirPlay
URL:=https://github.com/ejurgensen/forked-daapd
DEPENDS:=+libgpg-error +libgcrypt +libgdbm +zlib +libexpat +libunistring \
	+libevent2 +libdaemon +libantlr3c +confuse +alsa-lib +libffmpeg-full \
	+mxml +libavahi-client +sqlite3-cli +libplist +libcurl +libjson-c \
	+libprotobuf-c +libgnutls +libsodium +libwebsockets $(ICONV_DEPENDS) \
	+libuuid +libopus
endef

define Package/forked-daapd/description
  forked-daapd is a Linux/FreeBSD DAAP (iTunes), MPD (Music Player Daemon) and
  RSP (Roku) media server. It has support for AirPlay speakers, Chromecast,
  Apple Remote (and compatibles), MPD clients, internet radio and LastFM. It
  does not support AirPlay/Chromecast video.
endef

define Package/forked-daapd/conffiles
/etc/forked-daapd.conf
endef

CONFIGURE_ARGS += \
	--enable-lastfm \
	--enable-mpd \
	--enable-chromecast \
	--enable-verification \
	--enable-webinterface \
	--disable-spotify \
	--with-libplist \
	--with-libwebsockets \
	--with-alsa \
	--without-pulseaudio \
	--without-libevent_pthreads

TARGET_CFLAGS += $(FPIC)
TARGET_LDFLAGS += -Wl,-rpath-link,$(STAGING_DIR)/usr/lib

define Package/forked-daapd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/forked-daapd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./files/forked-daapd.conf $(1)/etc/forked-daapd.conf
	$(INSTALL_DIR) $(1)/usr/lib/forked-daapd
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/forked-daapd/* $(1)/usr/lib/forked-daapd/
	#$(INSTALL_DIR) $(1)/etc/init.d
	#$(INSTALL_BIN) ./files/forked-daapd.init $(1)/etc/init.d/forked-daapd
	$(INSTALL_DIR) $(1)/usr/share/forked-daapd/htdocs
	$(CP) $(PKG_BUILD_DIR)/htdocs/* $(1)/usr/share/forked-daapd/htdocs/
endef

$(eval $(call BuildPackage,forked-daapd))
