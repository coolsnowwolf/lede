#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=portaudio
PKG_VERSION:=190600_20161030
PKG_RELEASE:=2

PKG_SOURCE:=pa_stable_v$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://www.portaudio.com/archives/
PKG_HASH:=f5a21d7dcd6ee84397446fa1fa1a0675bb2e8a4a6dceb4305a8404698d8d1513
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_MAINTAINER:=
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.txt

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/portaudio
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+alsa-lib +libpthread +librt
  TITLE:=Portable cross-platform audio I/O
  URL:=http://www.portaudio.com/
endef

define Package/portaudio/description
 PortAudio is a free, cross-platform, open-source, audio I/O library.  It lets
 you write simple audio programs in 'C' or C++ that will compile and run on many
 platforms including Windows, Macintosh OS X, and Unix (OSS/ALSA). It is
 intended to promote the exchange of audio software between developers on
 different platforms. Many applications use PortAudio for Audio I/O.
endef

define Package/portaudio/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libportaudio.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,portaudio))
