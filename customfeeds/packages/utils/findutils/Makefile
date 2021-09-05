#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=findutils
PKG_VERSION:=4.8.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=57127b7e97d91282c6ace556378d5455a9509898297e46e10443016ea1387164

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnu:findutils

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/findutils/Default
	TITLE:=GNU Find Utilities
	SECTION:=utils
	CATEGORY:=Utilities
	URL:=https://www.gnu.org/software/findutils/
endef

define Package/findutils/description/Default
Replace busybox versions of findutils with full GNU versions.
This is normally not needed as busybox is smaller and provides
sufficient functionality, but some users may want or need
the full functionality of the GNU tools.
endef

define Package/findutils
	$(call Package/findutils/Default)
	TITLE+= (all)
	DEPENDS:= \
	+findutils-find \
	+findutils-xargs \
	+findutils-locate
endef

define Package/findutils-find
	$(call Package/findutils/Default)
	TITLE+= - find utility
	ALTERNATIVES:=300:/usr/bin/find:/usr/libexec/find-findutils
endef

define Package/findutils-xargs
	$(call Package/findutils/Default)
	TITLE+= - xargs utility
	ALTERNATIVES:=300:/usr/bin/xargs:/usr/libexec/xargs-findutils
endef

define Package/findutils-locate
	$(call Package/findutils/Default)
	TITLE+= - locate and updatedb utility
endef

CONFIGURE_ARGS += \
	--localstatedir=/srv/var \
	--without-selinux
CONFIGURE_VARS += ac_cv_path_SORT=sort

define Package/findutils/install
	true
endef

define Package/findutils-find/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/find $(1)/usr/libexec/find-findutils
endef

define Package/findutils-xargs/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/xargs $(1)/usr/libexec/xargs-findutils
endef

define Package/findutils-locate/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/srv/var
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/locate $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/updatedb $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib $(1)/usr/
endef

$(eval $(call BuildPackage,findutils))
$(eval $(call BuildPackage,findutils-find))
$(eval $(call BuildPackage,findutils-locate))
$(eval $(call BuildPackage,findutils-xargs))
