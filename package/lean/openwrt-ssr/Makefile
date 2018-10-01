#
# Copyright (C) 2017 OpenWrt-ssr
# Copyright (C) 2017 yushi studio <ywb94@qq.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-ssr
PKG_VERSION:=3.1.1
# PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/shadowsocksrr/shadowsocksr-libev
PKG_SOURCE_VERSION:=d4904568c0bd7e0861c0cbfeaa43740f404db214

PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Akkariiin

#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/openwrt-ssr/Default
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=shadowsocksR-libev LuCI interface
	URL:=https://github.com/MrTheUniverse/openwrt-ssr
	VARIANT:=$(1)
	DEPENDS:=$(3)	
	PKGARCH:=all
endef


Package/luci-app-shadowsocksR = $(call Package/openwrt-ssr/Default,openssl,(OpenSSL),+libopenssl +libpthread +ipset +ip-full +iptables-mod-tproxy +libpcre +zlib)
Package/luci-app-shadowsocksR-Client = $(call Package/openwrt-ssr/Default,openssl,(OpenSSL),+libopenssl +libpthread +ipset +ip-full +iptables-mod-tproxy +libpcre +zlib)
Package/luci-app-shadowsocksR-Server = $(call Package/openwrt-ssr/Default,openssl,(OpenSSL),+libopenssl +libpthread +ipset +ip-full +iptables-mod-tproxy +libpcre +zlib)
Package/luci-app-shadowsocksR-GFW = $(call Package/openwrt-ssr/Default,openssl,(OpenSSL),+libopenssl +libpthread +ipset +ip-full +iptables-mod-tproxy +libpcre +zlib +dnsmasq-full +coreutils +coreutils-base64 +curl +bash +bind-dig +pdnsd-alt)

define Package/openwrt-ssr/description
	LuCI Support for $(1).
endef

Package/luci-app-shadowsocksR/description = $(call Package/openwrt-ssr/description,shadowsocksr-libev Client and Server)
Package/luci-app-shadowsocksR-Client/description = $(call Package/openwrt-ssr/description,shadowsocksr-libev Client)
Package/luci-app-shadowsocksR-Server/description = $(call Package/openwrt-ssr/description,shadowsocksr-libev Server)
Package/luci-app-shadowsocksR-GFW/description = $(call Package/openwrt-ssr/description,shadowsocksr-libev GFW)

define Package/openwrt-ssr/prerm
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "Removing rc.d symlink for shadowsocksr"
     /etc/init.d/shadowsocksr disable
     /etc/init.d/shadowsocksr stop
    echo "Removing firewall rule for shadowsocksr"
	  uci -q batch <<-EOF >/dev/null
		delete firewall.shadowsocksr
		commit firewall
EOF
if [ "$(1)" = "GFW" ] ;then
sed -i '/conf-dir/d' /etc/dnsmasq.conf
/etc/init.d/dnsmasq restart 
fi
fi
exit 0
endef

Package/luci-app-shadowsocksR/prerm = $(call Package/openwrt-ssr/prerm,shadowsocksr)
Package/luci-app-shadowsocksR-Client/prerm = $(call Package/openwrt-ssr/prerm,shadowsocksr)
Package/luci-app-shadowsocksR-GFW/prerm = $(call Package/openwrt-ssr/prerm,GFW)

define Package/luci-app-shadowsocksR-Server/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
 /etc/init.d/shadowsocksr disable
 /etc/init.d/shadowsocksr stop
fi 
exit 0

endef


define Package/openwrt-ssr/postinst
#!/bin/sh

if [ -z "$${IPKG_INSTROOT}" ]; then
	uci -q batch <<-EOF >/dev/null
		delete firewall.shadowsocksr
		set firewall.shadowsocksr=include
		set firewall.shadowsocksr.type=script
		set firewall.shadowsocksr.path=/var/etc/shadowsocksr.include
		set firewall.shadowsocksr.reload=1
		commit firewall
EOF
fi

if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-shadowsocksr ) && rm -f /etc/uci-defaults/luci-shadowsocksr
	chmod 755 /etc/init.d/shadowsocksr >/dev/null 2>&1
	/etc/init.d/shadowsocksr enable >/dev/null 2>&1
    if [ "$(1)" = "GFW" ] ;then
	   if [ -f "/etc/dnsmasq.conf" ]; then
          str=`cat /etc/dnsmasq.conf|grep conf-dir`
          if [ -z "$str" ]; then
             echo "conf-dir=/etc/dnsmasq.ssr" >> /etc/dnsmasq.conf
         else
             sed -i '/conf-dir/d' /etc/dnsmasq.conf  
             echo "conf-dir=/etc/dnsmasq.ssr" >> /etc/dnsmasq.conf
           fi 
       else
           echo "conf-dir=/etc/dnsmasq.ssr" > /etc/dnsmasq.conf
       fi
	    /etc/init.d/dnsmasq restart 
	fi
fi
exit 0
endef


Package/luci-app-shadowsocksR/postinst = $(call Package/openwrt-ssr/postinst,shadowsocksr)
Package/luci-app-shadowsocksR-Client/postinst = $(call Package/openwrt-ssr/postinst,shadowsocksr)
Package/luci-app-shadowsocksR-GFW/postinst = $(call Package/openwrt-ssr/postinst,GFW)

define Package/luci-app-shadowsocksR-Server/postinst
#!/bin/sh

if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-shadowsocksr ) && rm -f /etc/uci-defaults/luci-shadowsocksr
	chmod 755 /etc/init.d/shadowsocksr >/dev/null 2>&1
	/etc/init.d/shadowsocksr enable >/dev/null 2>&1
fi
exit 0
endef



CONFIGURE_ARGS += --disable-documentation --disable-ssp

define Install/common
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/shadowsocksr.lua $(1)/usr/lib/lua/luci/controller/shadowsocksr.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) ./files/luci/i18n/shadowsocksr.*.lmo $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/shadowsocksr
	$(INSTALL_DATA) ./files/luci/model/cbi/shadowsocksr/*.lua $(1)/usr/lib/lua/luci/model/cbi/shadowsocksr/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/shadowsocksr
	$(INSTALL_DATA) ./files/luci/view/shadowsocksr/*.htm $(1)/usr/lib/lua/luci/view/shadowsocksr/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/root/etc/uci-defaults/luci-shadowsocksr $(1)/etc/uci-defaults/luci-shadowsocksr

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/shadowsocksr.config $(1)/etc/config/shadowsocksr
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/shadowsocksr.init $(1)/etc/init.d/shadowsocksr
endef

define Package/openwrt-ssr/install
	$(call Install/common,$(1))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-redir $(1)/usr/bin/ssr-redir
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-local $(1)/usr/bin/ssr-local	
	$(LN) /usr/bin/ssr-local $(1)/usr/bin/ssr-tunnel
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-server $(1)/usr/bin/ssr-server		
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-check $(1)/usr/bin/ssr-check
	$(INSTALL_BIN) ./files/shadowsocksr.rule $(1)/usr/bin/ssr-rules
	$(INSTALL_BIN) ./files/shadowsocksr.monitor $(1)/usr/bin/ssr-monitor
	$(INSTALL_BIN) ./files/shadowsocksr.switch $(1)/usr/bin/ssr-switch
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/china_ssr.txt $(1)/etc/china_ssr.txt
	$(INSTALL_DIR) $(1)/usr/share/shadowsocksr
	$(INSTALL_BIN) ./files/root/usr/share/shadowsocksr/*.sh $(1)/usr/share/shadowsocksr/
endef

Package/luci-app-shadowsocksR/install = $(call Package/openwrt-ssr/install,$(1),shadowsocksr)

define Package/luci-app-shadowsocksR-Client/install
	$(call Install/common,$(1))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-redir $(1)/usr/bin/ssr-redir
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-tunnel $(1)/usr/bin/ssr-tunnel	
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-local $(1)/usr/bin/ssr-local
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-check $(1)/usr/bin/ssr-check
	$(INSTALL_BIN) ./files/shadowsocksr.rule $(1)/usr/bin/ssr-rules
	$(INSTALL_BIN) ./files/shadowsocksr.monitor $(1)/usr/bin/ssr-monitor
	$(INSTALL_BIN) ./files/shadowsocksr.switch $(1)/usr/bin/ssr-switch
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/china_ssr.txt $(1)/etc/china_ssr.txt	
endef

define Package/luci-app-shadowsocksR-Server/install
	$(call Install/common,$(1))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-server $(1)/usr/bin/ssr-server		
	$(INSTALL_BIN) ./files/shadowsocksr.rule $(1)/usr/bin/ssr-rules
	$(INSTALL_BIN) ./files/shadowsocksr.monitor $(1)/usr/bin/ssr-monitor
endef

define Package/luci-app-shadowsocksR-GFW/install
	$(call Install/common,$(1))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-redir $(1)/usr/bin/ssr-redir
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-local $(1)/usr/bin/ssr-local	
	$(LN) /usr/bin/ssr-local $(1)/usr/bin/ssr-tunnel
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-server $(1)/usr/bin/ssr-server		
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-check $(1)/usr/bin/ssr-check
	$(INSTALL_BIN) ./files/shadowsocksr.rule $(1)/usr/bin/ssr-rules
	$(INSTALL_BIN) ./files/shadowsocksr.monitor $(1)/usr/bin/ssr-monitor
	$(INSTALL_BIN) ./files/shadowsocksr.gfw $(1)/usr/bin/ssr-gfw
	$(INSTALL_BIN) ./files/shadowsocksr.ad $(1)/usr/bin/ssr-ad
	$(INSTALL_BIN) ./files/shadowsocksr.switch $(1)/usr/bin/ssr-switch
	$(INSTALL_DIR) $(1)/etc/dnsmasq.ssr
	$(INSTALL_DATA) ./files/gfw_list.conf $(1)/etc/dnsmasq.ssr/gfw_list.conf
	$(INSTALL_DATA) ./files/custom_forward.conf $(1)/etc/dnsmasq.ssr/custom_forward.conf
	$(INSTALL_DATA) ./files/ad.conf $(1)/etc/dnsmasq.ssr/ad.conf
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/china_ssr.txt $(1)/etc/china_ssr.txt	
	$(INSTALL_DIR) $(1)/usr/share/shadowsocksr
	$(INSTALL_DATA) ./files/root/usr/share/shadowsocksr/*.sh $(1)/usr/share/shadowsocksr/
endef

#$(eval $(call BuildPackage,luci-app-shadowsocksR))
#$(eval $(call BuildPackage,luci-app-shadowsocksR-Client))
#$(eval $(call BuildPackage,luci-app-shadowsocksR-Server))
$(eval $(call BuildPackage,luci-app-shadowsocksR-GFW))
