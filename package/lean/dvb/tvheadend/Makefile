#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tvheadend
PKG_VERSION:=4.2.6
PKG_RELEASE:=1

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=c2a6b61d06ecf8c033f2f5be5746c146eb907d26
# This is stable git version...
# PKG_SOURCE_VERSION:=1a0262f5870e4fabd6b9c70a7bf2ebe420bc956e
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tvheadend/tvheadend.git
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE.md

PKG_FIXUP:=autoreconf

PKG_USE_MIPS16:=0

SHORT_GIT_VER:=$(shell echo $(PKG_SOURCE_VERSION) | cut -b -7)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/tvheadend
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Tvheadend is a TV streaming server for Linux
  DEPENDS:=+libopenssl +librt +zlib +libpcre +libdvbcsa +TVHEADEND_AVAHI_SUPPORT:libavahi-client $(ICONV_DEPENDS)
  URL:=https://tvheadend.org
  MAINTAINER:=Jan Čermák <jan.cermak@nic.cz>
endef

define Package/tvheadend/description
  Tvheadend is a TV streaming server and recorder for Linux, FreeBSD and Android
  supporting DVB-S, DVB-S2, DVB-C, DVB-T, ATSC, IPTV, SAT>IP and HDHomeRun as input sources.

  Tvheadend offers the HTTP (VLC, MPlayer), HTSP (Kodi, Movian) and SAT>IP streaming.
endef

define Package/tvheadend/config
  menu "Configuration"
  depends on PACKAGE_tvheadend
  source "$(SOURCE)/Config.in"
  endmenu
endef

ifeq ($(CONFIG_TVHEADEND_CWC_SUPPORT),)
  CONFIGURE_ARGS += --disable-cwc
endif

ifeq ($(CONFIG_TVHEADEND_LINUXDVB_SUPPORT),)
  CONFIGURE_ARGS += --disable-linuxdvb
endif

ifeq ($(CONFIG_TVHEADEND_DVBSCAN_SUPPORT),)
  CONFIGURE_ARGS += --disable-dvbscan
endif

ifeq ($(CONFIG_TVHEADEND_AVAHI_SUPPORT),)
  CONFIGURE_ARGS += --disable-avahi
else
  CONFIGURE_ARGS += --enable-avahi
endif

CONFIGURE_ARGS += \
	--arch=$(ARCH) \
	--disable-dbus_1 \
	--disable-libav \
	--enable-bundle \
	--enable-dvben50221 \
	--enable-tvhcsa \
	--enable-dvbcsa \
	--enable-inotify \
	--enable-epoll \
	--enable-trace \
	--nowerror \
        --disable-dvbscan \
        --disable-avahi \
        --disable-hdhomerun_static \
        --disable-pie \
        --disable-imagecache \
        --disable-ffmpeg_static \
        --disable-libx264 \
        --disable-libx264_static \
        --disable-libx265 \
        --disable-libx265_static \
        --disable-libvpx \
        --disable-libvpx_static \
        --disable-libtheora \
        --disable-libtheora_static \
        --disable-libvorbis \
        --disable-libvorbis_static \
        --disable-libfdkaac \
        --disable-libfdkaac_static \
        --disable-nvenc \
        --disable-libmfx_static \
        --disable-timeshift


define Build/Prepare
	$(call Build/Prepare/Default)
	echo 'Tvheadend $(shell echo $(PKG_VERSION)~$(SHORT_GIT_VER)-$(PKG_RELEASE))' \
	> $(PKG_BUILD_DIR)/debian/changelog
endef

define Package/conffiles
/etc/config/tvheadend
endef

define Package/tvheadend/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/tvheadend.init $(1)/etc/init.d/tvheadend
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/tvheadend.config $(1)/etc/config/tvheadend

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build.linux/tvheadend $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tvheadend))
