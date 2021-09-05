#
# Copyright (C) 2017 Chris Blake (chrisrblake93@gmail.com)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=beep
PKG_REV:=0d790fa45777896749a885c3b93b2c1476d59f20
PKG_VERSION:=1.3
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/johnath/beep.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_MIRROR_HASH:=fc97d41e68d7b7e43a972e3416dd9326acc40efe66856271ccb36e3678464dec

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/beep
  SECTION:=sound
  CATEGORY:=Sound
  DEPENDS:=+TARGET_x86:kmod-pcspkr @!TARGET_x86:kmod-gpio-beeper
  TITLE:=Play beep sounds through a PC speaker
  URL:=http://johnath.com/beep/README
endef

define Package/beep/description
	This program plays beeps through the PC speaker
endef

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)"

define Package/beep/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/beep $(1)/usr/bin
endef

$(eval $(call BuildPackage,beep))
