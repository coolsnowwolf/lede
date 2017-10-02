#
# Copyright (C) 2016 chenhw2 <chenhw2@github.com>
#
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-haproxy-tcp
PKG_VERSION:=0.1.4
PKG_RELEASE:=2

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=chenhw2 <chenhw2@github.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-haproxy-tcp
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for HAProxy-TCP
	PKGARCH:=all
	DEPENDS:=+haproxy-nossl
endef

define Package/luci-app-haproxy-tcp/description
	LuCI Support for HAProxy-TCP.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-haproxy-tcp/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ -f /etc/uci-defaults/luci-haproxy-tcp ]; then
		( . /etc/uci-defaults/luci-haproxy-tcp ) && \
		rm -f /etc/uci-defaults/luci-haproxy-tcp
	fi
	rm -rf /tmp/luci-indexcache /tmp/luci-modulecache
fi
exit 0
endef

define Package/luci-app-haproxy-tcp/prerm
#!/bin/sh
/etc/init.d/haproxy-tcp disable
/etc/init.d/haproxy-tcp stop
rm -f /usr/sbin/haproxy-tcp
exit 0
endef

define Package/luci-app-haproxy-tcp/conffiles
/etc/config/haproxy-tcp
endef

define Package/luci-app-haproxy-tcp/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/haproxy-tcp.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) ./files/luci/model/cbi/*.lua $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/root/etc/config/haproxy-tcp $(1)/etc/config/haproxy-tcp
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/root/etc/init.d/haproxy-tcp $(1)/etc/init.d/haproxy-tcp
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/root/etc/uci-defaults/luci-haproxy-tcp $(1)/etc/uci-defaults/luci-haproxy-tcp
	$(INSTALL_DIR) $(1)/usr/sbin
	$(LN) haproxy $(1)/usr/sbin/haproxy-tcp
endef

$(eval $(call BuildPackage,luci-app-haproxy-tcp))
