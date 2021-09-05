#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=less
PKG_VERSION:=563
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.greenwoodsoftware.com/less
PKG_HASH:=ce5b6d2b9fc4442d7a07c93ab128d2dff2ce09a1d4f2d055b95cf28dd0dc9a9a

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Julen Landa Alustiza <julen@zokormazo.info>
PKG_CPE_ID:=cpe:/a:gnu:less

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/less/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Pager program similar to more
  URL:=http://www.greenwoodsoftware.com/less/
  ALTERNATIVES:=200:/usr/bin/less:/usr/libexec/less-gnu
endef

define Package/less/Default/description
  Full version of GNU less utility
endef

define Package/less
  $(call Package/less/Default)
  DEPENDS:=+libncurses
  VARIANT:=narrow
endef

define Package/less/description
  $(call Package/less/Default/description)
endef

define Package/less-wide
  $(call Package/less/Default)
  TITLE+= (Unicode)
  DEPENDS:=+libncursesw
  VARIANT:=wide
endef

define Package/less-wide/description
  $(call Package/less/Default/description)
  This package contains the Unicode enabled version of less.
endef

ifeq ($(BUILD_VARIANT),narrow)
	CONFIGURE_VARS += \
		ac_cv_lib_ncursesw_initscr=no
endif

ifeq ($(BUILD_VARIANT),wide)
	CONFIGURE_VARS += \
		ac_cv_lib_ncursesw_initscr=yes
endif

define Package/less/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/less $(1)/usr/libexec/less-gnu
endef

Package/less-wide/install = $(Package/less/install)

$(eval $(call BuildPackage,less))
$(eval $(call BuildPackage,less-wide))
