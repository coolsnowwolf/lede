include $(TOPDIR)/rules.mk 

PKG_NAME:=luci-app-passwall-plus
PKG_VERSION:=1.2
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk


define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for FuckGFW Bash Script
	DEPENDS:=+bash +ipset +curl +jsonfilter +libpthread +libopenssl +luci-compat
# +iptables-mod-tproxy
	PKGARCH:=all
	MAINTAINER:=yiguihai
endef

define Package/$(PKG_NAME)/description
	This is probably the best proxy script.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
  /etc/config/passwall
  /etc/black.list
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/passwall.zh-cn.po $(1)/usr/lib/lua/luci/i18n/passwall.zh-cn.lmo
endef

define Package/$(PKG_NAME)/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
  /etc/init.d/passwall disable
  /etc/init.d/passwall stop  
  uci -q batch <<-EOF >/dev/null
	delete ucitrack.@passwall[-1]
	commit ucitrack
EOF
  rm -rf /tmp/luci*
fi
exit 0
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
  if [ ! -x /etc/init.d/passwall ]; then
    chmod +x /etc/init.d/passwall
  fi
  /etc/init.d/passwall enable
  rm -rf /tmp/luci*
fi
exit 0
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
