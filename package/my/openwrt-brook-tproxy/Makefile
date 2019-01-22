#
# Copyright (C) 2017 openwrt-ssr
# Copyright (C) 2017 yushi studio <ywb94@qq.com>
# Copyright (C) 2018 openwrt-brook-tproxy
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-brook-tproxy
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=WouldChar

PKG_BUILD_PARALLEL:=1
RELEASE_VERSION:=v20180909

ifeq ($(ARCH),mipsel)
BROOK_NAME:=brook_linux_mipsle
endif
ifeq ($(ARCH),mips)
BROOK_NAME:=brook_linux_mips
endif
ifeq ($(ARCH),i386)
BROOK_NAME:=brook_linux_386
endif
ifeq ($(ARCH),x86_64)
BROOK_NAME:=brook
endif
ifeq ($(ARCH),arm)
BROOK_NAME:=brook_linux_arm7
endif
ifeq ($(ARCH),aarch64)
BROOK_NAME:=brook_linux_arm64
endif

include $(INCLUDE_DIR)/package.mk

define Package/openwrt-brook-tproxy/Default
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=Brook LuCI interface
	URL:=https://github.com/WouldChar/openwrt-brook-tproxy
	DEPENDS:=$(1)
	PKGARCH:=all
endef

Package/openwrt-brook-tproxy = $(call Package/openwrt-brook-tproxy/Default,+ipset +ip +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64)

define Package/openwrt-brook-tproxy/description
	LuCI Support for Brook.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)

	wget -O $(PKG_BUILD_DIR)/brook https://github.com/txthinking/brook/releases/download/$(RELEASE_VERSION)/$(BROOK_NAME)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/openwrt-brook-tproxy/prerm
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
	echo "Removing rc.d symlink for brook"
	/etc/init.d/brook disable
	/etc/init.d/brook stop
	echo "Removing firewall rule for brook"
	uci -q batch <<-EOF >/dev/null
		delete firewall.brook
		commit firewall
EOF
	sed -i '/conf-dir/d' /etc/dnsmasq.conf
	/etc/init.d/dnsmasq restart

fi
exit 0
endef

define Package/openwrt-brook-tproxy/postinst
#!/bin/sh

if [ -z "$${IPKG_INSTROOT}" ]; then
	uci -q batch <<-EOF >/dev/null
		delete firewall.brook
		set firewall.brook=include
		set firewall.brook.type=script
		set firewall.brook.path=/var/etc/brook.include
		set firewall.brook.reload=1
		commit firewall
EOF
	( . /etc/uci-defaults/luci-brook ) && rm -f /etc/uci-defaults/luci-brook
	chmod 755 /etc/init.d/brook >/dev/null 2>&1
	/etc/init.d/brook enable >/dev/null 2>&1
fi
exit 0
endef

define Package/openwrt-brook-tproxy/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/brook.lua $(1)/usr/lib/lua/luci/controller/brook.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/brook.*.lmo $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/brook
	$(INSTALL_DATA) ./files/luci/model/cbi/brook/*.lua $(1)/usr/lib/lua/luci/model/cbi/brook/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/brook
	$(INSTALL_DATA) ./files/luci/view/brook/*.htm $(1)/usr/lib/lua/luci/view/brook/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/root/etc/uci-defaults/luci-brook $(1)/etc/uci-defaults/luci-brook
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/root/etc/config/brook $(1)/etc/config/brook
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/root/etc/init.d/brook $(1)/etc/init.d/brook
	$(INSTALL_DIR) $(1)/etc/dnsmasq.brook
	$(INSTALL_DATA) ./files/root/etc/dnsmasq.brook/gfw_list.conf $(1)/etc/dnsmasq.brook/gfw_list.conf
	$(INSTALL_DATA) ./files/root/etc/dnsmasq.brook/custom_list.conf $(1)/etc/dnsmasq.brook/custom_list.conf
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/root/etc/china_ip.txt $(1)/etc/china_ip.txt
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/root/usr/bin/brook-gfw $(1)/usr/bin/brook-gfw
	$(INSTALL_BIN) ./files/root/usr/bin/brook-ad $(1)/usr/bin/brook-ad
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/brook $(1)/usr/bin/brook
endef

$(eval $(call BuildPackage,openwrt-brook-tproxy))
