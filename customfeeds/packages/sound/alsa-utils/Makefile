#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-utils
PKG_VERSION:=1.2.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=ftp://ftp.alsa-project.org/pub/utils/ \
		http://distfiles.gentoo.org/distfiles/
PKG_HASH:=98ffc2d599de0577d423a48fa5f20a992ca0b82d812ed1f2e58ade49ac6da426

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/alsa-utils
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+alsa-lib +alsa-ucm-conf +libncursesw +libpthread
  TITLE:=ALSA (Advanced Linux Sound Architecture) utilities
  URL:=https://www.alsa-project.org/
endef

define Package/alsa-utils-seq
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+alsa-lib +libpthread
  TITLE:=ALSA sequencer utilities
  URL:=https://www.alsa-project.org/
endef

define Package/alsa-utils-tests
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=ALSA utilities test data (adds ~1.3M to image)
  DEPENDS:=+alsa-lib +libpthread
  URL:=https://www.alsa-project.org/
endef

CONFIGURE_ARGS += \
	--disable-rpath \
	--disable-alsatest \
	--disable-bat \
	--disable-xmlto \
	--disable-rst2man \
	--with-curses=ncursesw

CONFIGURE_VARS += \
	ac_cv_header_samplerate_h=no

TARGET_LDFLAGS += -latopology

define Package/alsa-utils/install
	$(INSTALL_DIR) $(1)/usr/{s,}bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/amixer $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/alsamixer $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aplay $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/arecord $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/alsactl $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/share/alsa/init
	$(INSTALL_DATA) \
	    $(PKG_INSTALL_DIR)/usr/share/alsa/init/* \
	    $(1)/usr/share/alsa/init/
endef

define Package/alsa-utils-seq/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aconnect $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/amidi $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aplaymidi $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/arecordmidi $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/iecset $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/alsaloop $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/alsatplg $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/alsaucm $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aseqdump $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aseqnet $(1)/usr/bin/
endef

define Package/alsa-utils-tests/install
	$(INSTALL_DIR) $(1)/usr/{s,}bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/speaker-test $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/sounds/alsa
	$(INSTALL_DATA) \
	    $(PKG_INSTALL_DIR)/usr/share/sounds/alsa/* \
	    $(1)/usr/share/sounds/alsa/
endef

$(eval $(call BuildPackage,alsa-utils))
$(eval $(call BuildPackage,alsa-utils-seq))
$(eval $(call BuildPackage,alsa-utils-tests))
