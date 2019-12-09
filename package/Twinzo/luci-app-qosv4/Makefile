#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-qosv4
PKG_VERSION:=1.1f
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-qosv4
  SECTION:=LuCI
  CATEGORY:=LuCI
  SUBMENU:=3. Applications
  TITLE:=LuCI Support for QoSv4.
  DEPENDS:=+tc +iptables-mod-conntrack-extra +iptables-mod-conntrack-extra +iptables-mod-filter +iptables-mod-imq +iptables-mod-ipopt +iptables-mod-nat-extra +iptables-mod-imq +kmod-sched
  PKGARCH:=all
  MAINTAINER:=qq 3341249
endef

define Package/luci-app-qosv4/description
An agent script that makes qosv4 configuration simple.
endef

define Build/Compile
endef

define Package/luci-app-qosv4/postinst
#!/bin/sh 

[ -n "${IPKG_INSTROOT}" ] || {
	( . /etc/uci-defaults/luci-qosv4 ) && rm -f /etc/uci-defaults/luci-qosv4
	chmod 755 /etc/init.d/qosv4  >/dev/null 2>&1
/etc/init.d/qosv4 enable  >/dev/null 2>&1	
sed -i -e '/qos_scheduler/d' /etc/crontabs/root >/dev/null 2>&1	
	exit 0
}
endef 

define Package/luci-app-qosv4/install
	$(CP) ./files/* $(1)
endef

$(eval $(call BuildPackage,luci-app-qosv4))
