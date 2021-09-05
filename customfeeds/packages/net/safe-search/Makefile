#
# Copyright (c) 2021 Gregory L. Dietsche <Gregory.Dietsche@cuw.edu>
# This is free software, licensed under the MIT License
#

include $(TOPDIR)/rules.mk

PKG_NAME:=safe-search
PKG_VERSION:=2.0.1
PKG_RELEASE:=$(AUTORELEASE)
PKG_LICENSE:=MIT
PKG_MAINTAINER:=Gregory L. Dietsche <Gregory.Dietsche@cuw.edu>

include $(INCLUDE_DIR)/package.mk

define Package/safe-search
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Safe Search
	PKGARCH:=all
endef

define Package/safe-search/description
This package prevents adult content from appearing in search results by
configuring dnsmasq to force all devices on your network to use Google and
Bing's Safe Search IP addresses. This is designed to be approperiate for most
businesses and families. The default filtering rules do not interfere with
normal web browsing.
endef

define Package/safe-search/conffiles
/etc/config/safe-search
endef

define Build/Compile
endef

define Package/safe-search/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(CP) ./files/safe-search.defaults $(1)/etc/uci-defaults/safe-search

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/safe-search.conf $(1)/etc/config/safe-search

	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/safe-search-update $(1)/usr/sbin/safe-search-update
	$(INSTALL_BIN) ./files/safe-search-maintenance $(1)/usr/sbin/safe-search-maintenance

	$(INSTALL_DIR) $(1)/etc/safe-search/enabled
	$(INSTALL_DIR) $(1)/etc/safe-search/available
	$(INSTALL_DATA) ./files/hosts/* $(1)/etc/safe-search/available/
endef

define Package/safe-search/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
  crontab -l | grep -v "safe-search-maintenance" | sort | uniq | crontab -
  echo "1 1 * * 1 /bin/nice /usr/sbin/safe-search-maintenance>/dev/null 2>&1">>/etc/crontabs/root
  /etc/init.d/cron restart
fi
exit 0
endef

define Package/safe-search/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	uci del_list dhcp.@dnsmasq[0].addnhosts=/etc/safe-search/enabled
	uci commit dhcp
	/etc/init.d/dnsmasq reload
	crontab -l | grep -v "safe-search-maintenance" | sort | uniq | crontab -
fi
exit 0
endef

define Package/safe-search/postrm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	rm -rf /etc/safe-search/enabled
	rmdir /etc/safe-search/available
	rmdir /etc/safe-search/
fi
exit 0
endef

$(eval $(call BuildPackage,safe-search))
