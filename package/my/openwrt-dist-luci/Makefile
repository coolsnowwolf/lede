#
# Copyright (C) 2014-2018 OpenWrt-dist
# Copyright (C) 2014-2018 Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-dist-luci
PKG_VERSION:=1.6.2
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jian Chang <aa65535@live.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Create/uci-defaults
	( \
		echo '#!/bin/sh'; \
		echo 'uci -q batch <<-EOF >/dev/null'; \
		echo "	delete ucitrack.@$(1)[-1]"; \
		echo "	add ucitrack $(1)"; \
		echo "	set ucitrack.@$(1)[-1].init=$(1)"; \
		echo '	commit ucitrack'; \
		echo 'EOF'; \
		echo 'rm -f /tmp/luci-indexcache'; \
		echo 'exit 0'; \
	) > $(PKG_BUILD_DIR)/luci-$(1)
endef

define Package/openwrt-dist-luci/Default
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=$(1) LuCI interface
	PKGARCH:=all
	DEPENDS:=$(2)
endef

Package/luci-app-chinadns = $(call Package/openwrt-dist-luci/Default,ChinaDNS,+ChinaDNS)
Package/luci-app-dns-forwarder = $(call Package/openwrt-dist-luci/Default,Dns-Forwarder,+dns-forwarder)
Package/luci-app-redsocks2 = $(call Package/openwrt-dist-luci/Default,RedSocks2,+redsocks2)
Package/luci-app-shadowvpn = $(call Package/openwrt-dist-luci/Default,ShadowVPN,+ShadowVPN)

define Package/openwrt-dist-luci/description
	LuCI Support for $(1).
endef

Package/luci-app-chinadns/description = $(call Package/openwrt-dist-luci/description,ChinaDNS)
Package/luci-app-dns-forwarder/description = $(call Package/openwrt-dist-luci/description,Dns-Forwarder)
Package/luci-app-redsocks2/description = $(call Package/openwrt-dist-luci/description,RedSocks2)
Package/luci-app-shadowvpn/description = $(call Package/openwrt-dist-luci/description,ShadowVPN)

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/openwrt-dist-luci/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-$(1) ) && rm -f /etc/uci-defaults/luci-$(1)
	chmod 755 /etc/init.d/$(1) >/dev/null 2>&1
	/etc/init.d/$(1) enable >/dev/null 2>&1
fi
exit 0
endef

Package/luci-app-chinadns/postinst = $(call Package/openwrt-dist-luci/postinst,chinadns)
Package/luci-app-dns-forwarder/postinst = $(call Package/openwrt-dist-luci/postinst,dns-forwarder)
Package/luci-app-redsocks2/postinst = $(call Package/openwrt-dist-luci/postinst,redsocks2)
Package/luci-app-shadowvpn/postinst = $(call Package/openwrt-dist-luci/postinst,shadowvpn)

define Package/openwrt-dist-luci/postrm
#!/bin/sh
rm -f /tmp/luci-indexcache
exit 0
endef

Package/luci-app-chinadns/postrm = $(Package/openwrt-dist-luci/postrm)
Package/luci-app-dns-forwarder/postrm = $(Package/openwrt-dist-luci/postrm)
Package/luci-app-redsocks2/postrm = $(Package/openwrt-dist-luci/postrm)
Package/luci-app-shadowvpn/postrm = $(Package/openwrt-dist-luci/postrm)

define Package/openwrt-dist-luci/install
	$(call Create/uci-defaults,$(2))
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/$(2).lua $(1)/usr/lib/lua/luci/controller/$(2).lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(2).*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) ./files/luci/model/cbi/$(2).lua $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/luci-$(2) $(1)/etc/uci-defaults/luci-$(2)
endef

Package/luci-app-chinadns/install = $(call Package/openwrt-dist-luci/install,$(1),chinadns)
Package/luci-app-dns-forwarder/install = $(call Package/openwrt-dist-luci/install,$(1),dns-forwarder)
Package/luci-app-redsocks2/install = $(call Package/openwrt-dist-luci/install,$(1),redsocks2)
Package/luci-app-shadowvpn/install = $(call Package/openwrt-dist-luci/install,$(1),shadowvpn)

$(eval $(call BuildPackage,luci-app-chinadns))
$(eval $(call BuildPackage,luci-app-dns-forwarder))
$(eval $(call BuildPackage,luci-app-redsocks2))
$(eval $(call BuildPackage,luci-app-shadowvpn))
