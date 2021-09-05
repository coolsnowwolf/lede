#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lrzsz
PKG_VERSION:=0.12.20
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://ohse.de/uwe/releases/
PKG_HASH:=c28b36b14bddb014d9e9c97c52459852f97bd405f89113f30bee45ed92728ff1

PKG_MAINTAINER:=Hsing-Wang Liao <kuoruan@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lrzsz
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=X, Y and Z-modem protocols
  URL:=https://ohse.de/uwe/software/lrzsz.html
endef

define Package/lrzsz/description
	Transfer files in your login sessions.
	Very leightweight and straight forward.
	You just need a terminal client that can do
	either X, Y or Z-modem file transfers.
endef

define Package/lrzsz/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lrz $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lsz $(1)/usr/bin/
	(cd $(1)/usr/bin; \
		ln -fs lrz lrx; \
		ln -fs lrz lrb; \
		ln -fs lrz rz; \
		ln -fs lrz rx; \
		ln -fs lrz rb; \
		ln -fs lsz lsx; \
		ln -fs lsz lsb; \
		ln -fs lsz sz; \
		ln -fs lsz sx; \
		ln -fs lsz sb; \
	);
endef

$(eval $(call BuildPackage,lrzsz))
