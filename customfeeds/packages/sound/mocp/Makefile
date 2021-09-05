#
# Copyright (C) 2011-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=moc
PKG_VERSION:=2.5.2
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://ftp.daper.net/pub/soft/moc/stable/
PKG_HASH:=f3a68115602a4788b7cfa9bbe9397a9d5e24c68cb61a57695d1c2c3ecf49db08

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/moc
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+libcurl $(ICONV_DEPENDS) +alsa-lib +libid3tag +libsamplerate +libmad +libfaad2 +libflac +libvorbis +libncursesw +libltdl +libmagic +libdb47
  TITLE:=Music On Console
  URL:=https://moc.daper.net/
endef

define Package/moc/description
	MOC (music on console) is a console audio player for LINUX/UNIX designed to be powerful and easy to use.
endef

TARGET_CPPFLAGS+=-P

CONFIGURE_ARGS += \
		--enable-shared \
		--disable-static \
		--disable-debug \
		--without-rcc \
		--without-oss \
		--without-sndio \
		--with-alsa \
		--without-jack \
		--with-magic \
		--without-ncurses \
		--with-ncursesw \
		--with-samplerate \
		--with-aac \
		--without-ffmpeg \
		--with-flac \
		--without-modplug \
		--with-mp3 \
		--without-musepack \
		--without-sidplay2 \
		--without-sndfile \
		--without-speex \
		--without-timidity \
		--with-vorbis \
		--without-wavpack \
		--with-curl \
		--with-bdb-dir="$(STAGING_DIR)/usr"

define Package/moc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mocp $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/moc/decoder_plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/moc/decoder_plugins/*.so $(1)/usr/lib/moc/decoder_plugins
endef

$(eval $(call BuildPackage,moc))
