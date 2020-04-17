include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-xlnetacc
PKG_VERSION:=1.0.3
PKG_RELEASE:=9

PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Sense <sensec@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for XLNetAcc
	PKGARCH:=all
	DEPENDS:=+jshn +wget +openssl-util
endef

define Package/$(PKG_NAME)/description
	LuCI Support for XLNetAcc.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-xlnetacc ) && rm -f /etc/uci-defaults/luci-xlnetacc
fi
exit 0
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/xlnetacc
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) ./luasrc/model/cbi/*.lua $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/xlnetacc
	$(INSTALL_DATA) ./luasrc/view/xlnetacc/*.htm $(1)/usr/lib/lua/luci/view/xlnetacc/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./root/etc/config/xlnetacc $(1)/etc/config/xlnetacc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/xlnetacc $(1)/etc/init.d/xlnetacc
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./root/etc/hotplug.d/iface/95-xlnetacc $(1)/etc/hotplug.d/iface/95-xlnetacc
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/luci-xlnetacc $(1)/etc/uci-defaults/luci-xlnetacc
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./root/usr/bin/xlnetacc.sh $(1)/usr/bin/xlnetacc.sh
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/xlnetacc.po $(1)/usr/lib/lua/luci/i18n/xlnetacc.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
