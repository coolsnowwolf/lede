#
# Copyright (C) 2013-2017 OpenWrt.org
#
include $(TOPDIR)/rules.mk

PKG_NAME:=prometheus-node-exporter-lua
PKG_VERSION:=2021.02.15
PKG_RELEASE:=1

PKG_MAINTAINER:=Etienne CHAMPETIER <champetier.etienne@gmail.com>
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk

Build/Compile=

define Package/prometheus-node-exporter-lua/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Prometheus node exporter
  PKGARCH:=all
endef

define Package/prometheus-node-exporter-lua
  $(call Package/prometheus-node-exporter-lua/Default)
  DEPENDS:=+luasocket +lua
endef

define Package/prometheus-node-exporter-lua/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/prometheus-node-exporter-lua $(1)/etc/config/prometheus-node-exporter-lua
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/prometheus-node-exporter-lua $(1)/etc/init.d/prometheus-node-exporter-lua
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/prometheus-node-exporter-lua $(1)/usr/bin/prometheus-node-exporter-lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/cpu.lua         $(1)/usr/lib/lua/prometheus-collectors/
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/conntrack.lua   $(1)/usr/lib/lua/prometheus-collectors/
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/filefd.lua      $(1)/usr/lib/lua/prometheus-collectors/
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/loadavg.lua     $(1)/usr/lib/lua/prometheus-collectors/
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/meminfo.lua     $(1)/usr/lib/lua/prometheus-collectors/
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/netdev.lua      $(1)/usr/lib/lua/prometheus-collectors/
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/time.lua        $(1)/usr/lib/lua/prometheus-collectors/
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/uname.lua       $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua/conffiles
/etc/config/prometheus-node-exporter-lua
endef

define Package/prometheus-node-exporter-lua/description
  Provides node metrics as Prometheus scraping endpoint.

  This service is a lightweight rewrite in LUA of the offical Prometheus node_exporter.
endef

# Additional optional exporters:

define Package/prometheus-node-exporter-lua-bmx6
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (bmx6 links collector)
  DEPENDS:=prometheus-node-exporter-lua bmx6 +lua-cjson +bmx6-json
endef

define Package/prometheus-node-exporter-lua-bmx6/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/bmx6.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-bmx7
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (bmx7 links collector)
  DEPENDS:=prometheus-node-exporter-lua bmx7 +lua-cjson +bmx7-json
endef

define Package/prometheus-node-exporter-lua-bmx7/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/bmx7.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-dawn
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (dawn collector)
  DEPENDS:=prometheus-node-exporter-lua +libiwinfo-lua +libubus-lua
endef

define Package/prometheus-node-exporter-lua-dawn/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/dawn.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-hostapd_stations
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (hostapd_stations collector) - Requires a full hostapd / wpad build
  DEPENDS:=prometheus-node-exporter-lua +hostapd-utils +lua-bit32 +libubus-lua
endef

define Package/prometheus-node-exporter-lua-hostapd_stations/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/hostapd_stations.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-hostapd_ubus_stations
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (hostapd_ubus_stations collector)
  DEPENDS:=prometheus-node-exporter-lua +luabitop +libubus-lua
endef

define Package/prometheus-node-exporter-lua-hostapd_ubus_stations/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/hostapd_ubus_stations.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-ltq-dsl
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (lantiq dsl collector)
  DEPENDS:=prometheus-node-exporter-lua @(PACKAGE_ltq-adsl-app||PACKAGE_ltq-vdsl-app)
endef

define Package/prometheus-node-exporter-lua-ltq-dsl/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/ltq-dsl.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-nat_traffic
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (nat_traffic collector)
  DEPENDS:=prometheus-node-exporter-lua
endef

define Package/prometheus-node-exporter-lua-nat_traffic/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/nat_traffic.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-netstat
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (netstat collector)
  DEPENDS:=prometheus-node-exporter-lua
endef

define Package/prometheus-node-exporter-lua-netstat/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/netstat.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-openwrt
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (openwrt collector)
  DEPENDS:=prometheus-node-exporter-lua +libubus-lua
endef

define Package/prometheus-node-exporter-lua-openwrt/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/openwrt.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-textfile
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (textfile collector)
  DEPENDS:=prometheus-node-exporter-lua +luci-lib-nixio
endef

define Package/prometheus-node-exporter-lua-textfile/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/textfile.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-uci_dhcp_host
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (uci_dhcp_host collector)
  DEPENDS:=prometheus-node-exporter-lua +libuci-lua
endef

define Package/prometheus-node-exporter-lua-uci_dhcp_host/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/uci_dhcp_host.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-wifi
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (wifi collector)
  DEPENDS:=prometheus-node-exporter-lua +libiwinfo-lua +libubus-lua
endef

define Package/prometheus-node-exporter-lua-wifi/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/wifi.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-wifi_stations
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (wifi_stations collector)
  DEPENDS:=prometheus-node-exporter-lua +libiwinfo-lua +libubus-lua
endef

define Package/prometheus-node-exporter-lua-wifi_stations/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/wifi_stations.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

define Package/prometheus-node-exporter-lua-snmp6
  $(call Package/prometheus-node-exporter-lua/Default)
  TITLE+= (snmp6 collector)
  DEPENDS:=prometheus-node-exporter-lua +libubus-lua
endef

define Package/prometheus-node-exporter-lua-snmp6/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/prometheus-collectors
	$(INSTALL_BIN) ./files/usr/lib/lua/prometheus-collectors/snmp6.lua $(1)/usr/lib/lua/prometheus-collectors/
endef

$(eval $(call BuildPackage,prometheus-node-exporter-lua))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-bmx6))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-bmx7))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-dawn))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-hostapd_stations))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-hostapd_ubus_stations))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-ltq-dsl))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-nat_traffic))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-netstat))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-openwrt))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-textfile))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-uci_dhcp_host))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-wifi))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-wifi_stations))
$(eval $(call BuildPackage,prometheus-node-exporter-lua-snmp6))
