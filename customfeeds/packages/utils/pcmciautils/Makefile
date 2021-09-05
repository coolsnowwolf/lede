#
# Copyright (C) 2008-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pcmciautils
PKG_VERSION:=018
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@KERNEL/linux/utils/kernel/pcmcia
PKG_HASH:=79e6ae441278e178c07501d492394ed2c0326fdb66894f6d040ec811b0dc8ed5

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_MAINATINER:=Tomasz Maciej Nowak <tomek_n@o2.pl>

include $(INCLUDE_DIR)/package.mk

define Package/pcmciautils
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libsysfs @PCMCIA_SUPPORT
  TITLE:=PCMCIA Utilities
  URL:=http://www.kernel.org/pub/linux/utils/kernel/pcmcia/pcmcia.html
endef

define Build/Compile
	$(call Build/Compile/Default, \
		LD="$(TARGET_CC)" UDEV=false \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		-DPCMCIAUTILS_VERSION=\\\"$(PKG_VERSION)\\\"" \
		prefix=/usr all \
	)
endef

define Package/pcmciautils/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/pccardctl $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/pcmcia-check-broken-cis $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/pcmcia-socket-startup $(1)/usr/sbin/
	rm -f $(1)/usr/sbin/lspcmcia
	ln -s pccardctl $(1)/usr/sbin/lspcmcia
	$(INSTALL_DIR) $(1)/etc/pcmcia/
	$(CP) $(PKG_BUILD_DIR)/config/config.opts $(1)/etc/pcmcia/
	$(CP) files/* $(1)/
endef

define Package/pcmciautils/conffiles
/etc/pcmcia/config.opts
endef

$(eval $(call BuildPackage,pcmciautils))
