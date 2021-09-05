#
# Copyright (C) 2012-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bluelog
PKG_VERSION:=1.1.2
PKG_RELEASE:=4

PKG_SOURCE:=Bluelog-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/MS3FGX/Bluelog/tar.gz/$(PKG_VERSION)?
PKG_HASH:=ebbc1357e14bc46cbddd8390cdbd29c0131b09b8ab680a1c382164ef076cb53e
PKG_BUILD_DIR:=$(BUILD_DIR)/Bluelog-$(PKG_VERSION)

OUI_SOURCE:=oui-2016-05-30.txt.gz
OUI_URL:=https://sources.openwrt.org/
OUI_HASH:=19d0b02eeb0d42507ee17b637a295782c14956fe1ac4803589ed93ef92c4fad5

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Nicolas Thill <nico@openwrt.org>

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/bluelog/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Bluetooth scanner and logger
  URL:=https://github.com/MS3FGX/Bluelog
  DEPENDS:=+bluez-libs +kmod-bluetooth
endef

define Package/bluelog/Default/description
  Bluelog is a simple Bluetooth scanner designed to tell you how many
  discoverable devices there are in an area as quickly as possible. It is
  intended to be used as a site survey tool, identifying the number of possible
  Bluetooth targets there are in the surrounding environment.
endef

define Package/bluelog
  $(call Package/bluelog/Default)
endef

define Package/bluelog/description
  $(call Package/bluelog/Default/description)
endef

define Package/bluelog-live
  $(call Package/bluelog/Default)
  TITLE+= (live output)
  DEPENDS+= bluelog
endef

define Package/bluelog-live/description
  $(call Package/bluelog/Default/description)
  This package contains the files for "Bluelog Live", an optional mode of
  Bluelog which creates a real-time webpage of discovered Bluetooth devices.
endef

define Download/oui.txt
  FILE:=$(OUI_SOURCE)
  URL:=$(OUI_URL)
  HASH:=$(OUI_HASH)
endef

TARGET_CFLAGS += -DOPENWRT

MAKE_FLAGS += \
	 LIBS="$(TARGET_LDFLAGS) -lbluetooth -lm"

define Build/Prepare
	$(eval $(call Download,oui.txt))
	$(Build/Prepare/Default)
	zcat $(DL_DIR)/$(OUI_SOURCE) > $(PKG_BUILD_DIR)/oui.tmp
endef

define Package/bluelog/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bluelog $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/bluelog.init $(1)/etc/init.d/bluelog
endef

define Package/bluelog-live/install
	$(INSTALL_DIR) $(1)/www/bluelog
	$(CP) $(PKG_INSTALL_DIR)/usr/share/bluelog/*.html $(1)/www/bluelog/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/bluelog/openwrt.css \
		$(1)/www/bluelog/style.css
	$(INSTALL_DIR) $(1)/www/bluelog/images
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/bluelog/images/digifail_logo.png \
		$(PKG_INSTALL_DIR)/usr/share/bluelog/images/email.png \
		$(PKG_INSTALL_DIR)/usr/share/bluelog/images/favicon.png \
		$(PKG_INSTALL_DIR)/usr/share/bluelog/images/openwrt_logo.png \
		$(PKG_INSTALL_DIR)/usr/share/bluelog/images/qrcontact.png \
		$(1)/www/bluelog/images/
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(CP) $(PKG_INSTALL_DIR)/usr/share/bluelog/cgi-bin/* $(1)/www/cgi-bin/
endef

$(eval $(call BuildPackage,bluelog))
$(eval $(call BuildPackage,bluelog-live))
