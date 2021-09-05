#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sox
PKG_VERSION:=14.4.2
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/sox
PKG_HASH:=81a6956d4330e75b5827316e44ae381e6f1e8928003c6aa45896da9041ea149c

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=LGPL-2.1 GPL-2.0
PKG_LICENSE_FILES:=COPYING LICENSE.LGPL LICENSE.GPL


PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

TARGET_LDFLAGS+= \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/sox
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+lame-lib +libmad +libid3tag +libmagic \
		+libvorbis +alsa-lib +libflac
  TITLE:=Sox is a general purpose sound converter/player/recorder
  URL:=http://sox.sourceforge.net/
endef

define Package/sox/description
	SoX is a command line utility that can convert various formats
	of computer audio files in to other formats. It can also apply
	various effects to these sound files during the conversion.
	As an added bonus, SoX can play and record audio files on
	several unix-style platforms.
endef

CONFIGURE_ARGS += \
		--without-oss \
		--without-ao \
		--with-alsa \
		--without-libltdl \
		--with-flac \
		--without-ladspa \
		--without-png \
		--without-sndfile \
		--without-opus \
		--with-lame \
		--with-id3tag \
		--disable-openmp

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include \
		$(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libsox.{a,so*,la} \
		$(1)/usr/lib/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* \
		$(1)/usr/lib/pkgconfig/
endef

define Package/sox/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/{play,rec,sox} $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsox.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,sox))
