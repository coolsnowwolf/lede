#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tracertools
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/dangowrt/tracertools
PKG_SOURCE_DATE:=2020-08-16
PKG_SOURCE_VERSION:=d8acfcad1fdf44cd76b8ba9014326f0db6e582d5
PKG_MIRROR_HASH:=db0748a58385aa600d147d0647ca3261d9407b9004db11d401977c59f93399ff

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tracertools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Tracer MPPT monitoring and control tools
endef

define Package/tracertools/description
	Tools for the Tracer MPPT solar charge controller.
endef

define Package/tracertools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tracerstat $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tracertools))
