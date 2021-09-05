#
# Copyright (C) 2017 Lucian Cristian <lucian.cristian@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=greyfix
PKG_VERSION:=0.4.0
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://www.kim-minh.com/pub/greyfix
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=26013edce3a38d586282bfc22eb91bd22df54e3558ea1b3dae54d3e7a769e4fe

MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE:=GPLv2

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/greyfix
  SECTION:=mail
  CATEGORY:=Mail
  DEPENDS:=+libdb47
  TITLE:=Greyfix - greylisting with Postfix.
  URL:=http://www.kim-minh.com/pub/greyfix/
endef

define Package/greyfix/description
	Greyfix is the greylisting policy daemon for Postfix written by Kim Minh Kaplan.
endef

CONFIGURE_ARGS += \
	--localstatedir=/usr/lib \

define Package/greyfix/install
	$(INSTALL_DIR)  $(1)/usr/sbin \
			$(1)/usr/lib/greyfix
	$(CP) $(PKG_BUILD_DIR)/greyfix $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,greyfix))
