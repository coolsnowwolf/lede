#
# Copyright (C) 2016-2017 Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-mentohust
PKG_VERSION:=4.0.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Chen Minqiang <ptpt52@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-mentohust
	CATEGORY:=Ptpt52
	SUBMENU:=Mentohust
	TITLE:=LuCI Support for mentohust
	PKGARCH:=all
	DEPENDS:=+mentohust
endef

define Package/luci-app-mentohust/description
	LuCI Support for mentohust.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-mentohust/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	rm -rf /tmp/luci-indexcache /tmp/luci-modulecache
fi
exit 0
endef

define Package/luci-app-mentohust/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mentohust.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/mentohust
	$(INSTALL_DATA) ./files/luci/model/cbi/mentohust/*.lua $(1)/usr/lib/lua/luci/model/cbi/mentohust/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/mentohust
	$(INSTALL_DATA) ./files/luci/view/mentohust/*.htm $(1)/usr/lib/lua/luci/view/mentohust/
endef

$(eval $(call BuildPackage,luci-app-mentohust))
