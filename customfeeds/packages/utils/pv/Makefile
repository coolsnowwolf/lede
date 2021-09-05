#
# Copyright (C) 2008 David Cooper
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pv
PKG_VERSION:=1.6.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.ivarch.com/programs/sources
PKG_HASH:=608ef935f7a377e1439c181c4fc188d247da10d51a19ef79bcdee5043b0973f1
PKG_LICENSE:=Artistic-2.0
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/pv
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Shell pipeline element to meter data passing through
  URL:=https://www.ivarch.com/programs/pv.shtml
endef

define Package/pv/description
 Pipe Viewer is a terminal-based tool for monitoring the progress of data
 through a pipeline. It can be inserted into any normal pipeline between
 two processes to give a visual indication of how quickly data is passing
 through, how long it has taken, how near to completion it is, and an
 estimate of how long it will be until completion.
endef

define Package/pv/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pv $(1)/usr/bin/
endef

$(eval $(call BuildPackage,pv))

