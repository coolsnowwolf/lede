#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=svox
PKG_VERSION:=1.0+git20130326
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/non-free/s/svox
PKG_HASH:=337b25e6ccb3764f0df1e176470b883c90e40e98840d4133340fcc89eb3cea0c

PKG_MAINTAINER:=Alessandro Di Marco <dmr@ethzero.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/svox
  TITLE:=SVOX PicoTTS text-to-speech engine
  SECTION:=sound
  CATEGORY:=Sound
  URL:=https://android.googlesource.com/platform/external/svox/
  DEPENDS:=+libpopt
endef

define Package/svox/description
  SVOX is an embedded speech technology company founded in 2000 and
  headquartered in Zurich, Switzerland. SVOX was acquired by Nuance
  Communications in 2011. Company's products included Automated Speech
  Recognition (ASR), Text-to-Speech (TTS) and Speech Dialog systems,
  with customers mostly being manufacturers and system integrators in
  automotive and mobile device industries.
  SVOX TTS technology is characterized by natural and clear sound as well
  as unique polyglot capability - the same voice can speak multiple
  languages like a native speaker.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
ifeq ($(QUILT),)
	mv $(PKG_BUILD_DIR)/pico/* $(PKG_BUILD_DIR)
endif
endef

define Package/svox/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pico2wave $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libttspico.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/share/pico/lang
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/pico/lang/* $(1)/usr/share/pico/lang/
endef

$(eval $(call BuildPackage,svox))
