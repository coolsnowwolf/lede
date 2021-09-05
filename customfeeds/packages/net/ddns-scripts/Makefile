#
# Copyright (C) 2008-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ddns-scripts
# Version == major.minor.patch
# increase on new functionality (minor) or patches (patch)
PKG_VERSION:=2.7.8
# Release == build
# increase on changes of services files or tld_names.dat
PKG_RELEASE:=3

PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=

include $(INCLUDE_DIR)/package.mk

# no default dependencies
PKG_DEFAULT_DEPENDS=

define Package/ddns-scripts/Default
    SECTION:=net
    CATEGORY:=Network
    SUBMENU:=IP Addresses and Names
    PKGARCH:=all
endef

###### *************************************************************************
define Package/ddns-scripts
    $(call Package/ddns-scripts/Default)
    TITLE:=Dynamic DNS Client scripts (with IPv6 support)
endef
# shown in LuCI package description
define Package/ddns-scripts/description
    Dynamic DNS Client scripts (with IPv6 support) - Info: http://wiki.openwrt.org/doc/howto/ddns.client
endef
# shown in menuconfig <Help>
define Package/ddns-scripts/config
	help
		A highly configurable set of scripts for doing dynamic dns updates.
		  - IPv6 support
		  - DNS server support
		  - Glue Record support (require BIND host or KNOT host)
		  - DNS requests via TCP
		  - Proxy server support
		  - log file support
		  - support to run once
		Version: $(PKG_VERSION)-$(PKG_RELEASE)
		Info   : http://wiki.openwrt.org/doc/howto/ddns.client
endef

###### *************************************************************************
define Package/ddns-scripts_cloudflare.com-v4
    $(call Package/ddns-scripts/Default)
    TITLE:=CloudFlare.com API v4 (requires cURL)
    DEPENDS:=ddns-scripts +curl
endef
define Package/ddns-scripts_cloudflare.com-v4/description
    Dynamic DNS Client scripts extension for CloudFlare.com API-v4 (require/install cURL)
endef

###### *************************************************************************
define Package/ddns-scripts_freedns_42_pl
    $(call Package/ddns-scripts/Default)
    TITLE:=DDNS extension for FreeDNS.42.pl (requires cURL)
    DEPENDS:=ddns-scripts +curl
endef
define Package/ddns-scripts_freedns_42_pl/description
    Dynamic DNS Client scripts extension for freedns.42.pl
endef

###### *************************************************************************
define Package/ddns-scripts_godaddy.com-v1
    $(call Package/ddns-scripts/Default)
    TITLE:=GoDaddy.com (require cURL)
    DEPENDS:=ddns-scripts +curl
endef
define Package/ddns-scripts_godaddy.com-v1/description
    Dynamic DNS Client scripts extension for GoDaddy.com (require/install cURL)
endef

###### *************************************************************************
define Package/ddns-scripts_no-ip_com
    $(call Package/ddns-scripts/Default)
    TITLE:=DDNS extension for No-IP.com
    DEPENDS:=ddns-scripts
endef
define Package/ddns-scripts_no-ip_com/description
    Dynamic DNS Client scripts extension for No-IP.com
endef

###### *************************************************************************
define Package/ddns-scripts_nsupdate
    $(call Package/ddns-scripts/Default)
    TITLE:=DDNS extension using Bind nsupdate
    DEPENDS:=ddns-scripts +bind-client
endef
define Package/ddns-scripts_nsupdate/description
    Dynamic DNS Client scripts extension for direct updates using Bind nsupdate
endef
define Package/ddns-scripts_nsupdate/config
    help
	The script directly updates a PowerDNS (or maybe bind server) via nsupdate
	from bind-client package. It requires
	"option dns_server" to be set to the server to be used by nsupdate.
	"option username" should be set to the key name and
	"option password" to the base64 encoded shared secret.

endef

###### *************************************************************************
define Package/ddns-scripts_route53-v1
    $(call Package/ddns-scripts/Default)
    TITLE:=Amazon AWS Route 53 API v1
    DEPENDS:=ddns-scripts +curl +openssl-util
endef
define Package/ddns-scripts_route53-v1/description
	Dynamic DNS Client scripts extension for Amazon AWS Route53. Note: You
	must also install ca-certificate or ca-bundle.
	It requires:
	"option username" to be a valid AWS access key id
	"option password" to be the matching AWS secret key id
	"option domain" to contain the hosted zone ID
endef

###### *************************************************************************
define Build/Configure
endef
define Build/Compile
	$(CP) ./files $(PKG_BUILD_DIR)
	# ensure that VERSION inside dynamic_dns_functions.sh reflect PKG_VERSION of Makefile
	$(SED) '/^VERSION=*/s/.*/VERSION="$(PKG_VERSION)-$(PKG_RELEASE)"/' $(PKG_BUILD_DIR)/files/dynamic_dns_functions.sh
	# remove comments, white spaces and empty lines
	for FILE in `find $(PKG_BUILD_DIR)/files -type f`; do \
		$(SED) 's/^[[:space:]]*//' \
		-e '/^#[[:space:]]\|^#$$$$/d' \
		-e 's/[[:space:]]#[[:space:]].*$$$$//' \
		-e 's/[[:space:]]*$$$$//' \
		-e '/^\/\/[[:space:]]/d'	\
		-e '/^[[:space:]]*$$$$/d'	$$$$FILE; \
	done
endef

define Package/ddns-scripts/conffiles
/etc/config/ddns
endef

###### *************************************************************************
define Package/ddns-scripts/preinst
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0	# suppress errors
endef
define Package/ddns-scripts/install
	$(INSTALL_DIR)  $(1)/etc/uci-defaults
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/files/ddns.defaults $(1)/etc/uci-defaults/ddns
	$(INSTALL_DIR)  $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/files/ddns.hotplug $(1)/etc/hotplug.d/iface/95-ddns
	$(INSTALL_DIR)  $(1)/etc/init.d
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/files/ddns.init $(1)/etc/init.d/ddns
	$(INSTALL_DIR)  $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/ddns.config $(1)/etc/config/ddns

	$(INSTALL_DIR)  $(1)/etc/ddns
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/files/services* $(1)/etc/ddns

	$(INSTALL_DIR)  $(1)/usr/lib/ddns
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/files/dynamic_dns_*.sh $(1)/usr/lib/ddns
endef
define Package/ddns-scripts/postinst
	#!/bin/sh
	# if NOT run buildroot and PKG_UPGRADE then (re)start service if enabled
	[ -z "$${IPKG_INSTROOT}" -a "$${PKG_UPGRADE}" = "1" ] && {
		[ -x /etc/uci-defaults/ddns ] && \
			/etc/uci-defaults/ddns && \
				rm -f /etc/uci-defaults/ddns >/dev/null 2>&1
		/etc/init.d/ddns enabled && \
			/etc/init.d/ddns start >/dev/null 2>&1
	}
	exit 0	# suppress errors
endef
define Package/ddns-scripts/prerm
	#!/bin/sh
	# if run within buildroot exit
	[ -n "$${IPKG_INSTROOT}" ] && exit 0
	# stop running scripts
	/etc/init.d/ddns stop
	/etc/init.d/ddns disable
	# clear LuCI indexcache
	rm -f /tmp/luci-indexcache >/dev/null 2>&1
	exit 0	# suppress errors
endef

###### *************************************************************************
define Package/ddns-scripts_cloudflare.com-v4/preinst
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0	# suppress errors
endef
define Package/ddns-scripts_cloudflare.com-v4/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ddns.defaults $(1)/etc/uci-defaults/ddns_cloudflare.com-v4
	$(INSTALL_DIR) $(1)/usr/lib/ddns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/update_cloudflare_com_v4.sh $(1)/usr/lib/ddns
endef
define Package/ddns-scripts_cloudflare.com-v4/postinst
	#!/bin/sh
	# remove old services file entries
	/bin/sed -i '/cloudflare\.com-v4/d' $${IPKG_INSTROOT}/etc/ddns/services		>/dev/null 2>&1
	/bin/sed -i '/cloudflare\.com-v4/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6	>/dev/null 2>&1
	# and create new
	printf "%s\\t%s\\n" '"cloudflare.com-v4"' '"update_cloudflare_com_v4.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services
	printf "%s\\t%s\\n" '"cloudflare.com-v4"' '"update_cloudflare_com_v4.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services_ipv6
	# on real system restart service if enabled
	[ -z "$${IPKG_INSTROOT}" ] && {
		[ -x /etc/uci-defaults/ddns_cloudflare.com-v4 ] && \
			/etc/uci-defaults/ddns_cloudflare.com-v4 && \
				rm -f /etc/uci-defaults/ddns_cloudflare.com-v4 >/dev/null 2>&1
		/etc/init.d/ddns enabled && \
			/etc/init.d/ddns start >/dev/null 2>&1
	}
	exit 0	# suppress errors
endef
define Package/ddns-scripts_cloudflare.com-v4/prerm
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop				>/dev/null 2>&1
	# remove services file entries
	/bin/sed -i '/cloudflare\.com-v4/d' $${IPKG_INSTROOT}/etc/ddns/services		>/dev/null 2>&1
	/bin/sed -i '/cloudflare\.com-v4/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6	>/dev/null 2>&1
	exit 0	# suppress errors
endef

###### *************************************************************************
define Package/ddns-scripts_freedns_42_pl/preinst
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0	# suppress errors
endef
define Package/ddns-scripts_freedns_42_pl/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ddns.defaults $(1)/etc/uci-defaults/ddns_freedns_42_pl
	$(INSTALL_DIR) $(1)/usr/lib/ddns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/update_freedns_42_pl.sh $(1)/usr/lib/ddns
endef
define Package/ddns-scripts_freedns_42_pl/postinst
	#!/bin/sh
	# remove old services file entries
	/bin/sed -i '/freedns\.42\.pl/d' $${IPKG_INSTROOT}/etc/ddns/services	>/dev/null 2>&1
	# and create new
	printf "%s\\t%s\\n" '"freedns.42.pl"' '"update_freedns_42_pl.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services
	# on real system restart service if enabled
	[ -z "$${IPKG_INSTROOT}" ] && {
		[ -x /etc/uci-defaults/ddns_freedns_42_pl ] && \
			/etc/uci-defaults/ddns_freedns_42_pl && \
				rm -f /etc/uci-defaults/ddns_freedns_42_pl >/dev/null 2>&1
		/etc/init.d/ddns enabled && \
			/etc/init.d/ddns start >/dev/null 2>&1
	}
	exit 0	# suppress errors
endef
define Package/ddns-scripts_freedns_42_pl/prerm
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop		>/dev/null 2>&1
	# remove services file entries
	/bin/sed -i '/freedns\.42\.pl/d' $${IPKG_INSTROOT}/etc/ddns/services	>/dev/null 2>&1
	exit 0	# suppress errors
endef

###### *************************************************************************
define Package/ddns-scripts_godaddy.com-v1/preinst
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0	# suppress errors
endef
define Package/ddns-scripts_godaddy.com-v1/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ddns.defaults $(1)/etc/uci-defaults/ddns_godaddy.com-v1
	$(INSTALL_DIR) $(1)/usr/lib/ddns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/update_godaddy_com_v1.sh $(1)/usr/lib/ddns
endef
define Package/ddns-scripts_godaddy.com-v1/postinst
	#!/bin/sh
	# remove old services file entries
	/bin/sed -i '/godaddy\.com-v1/d' $${IPKG_INSTROOT}/etc/ddns/services		>/dev/null 2>&1
	/bin/sed -i '/godaddy\.com-v1/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6	>/dev/null 2>&1
	# and create new
	printf "%s\\t%s\\n" '"godaddy.com-v1"' '"update_godaddy_com_v1.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services
	printf "%s\\t%s\\n" '"godaddy.com-v1"' '"update_godaddy_com_v1.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services_ipv6
	# on real system restart service if enabled
	[ -z "$${IPKG_INSTROOT}" ] && {
		[ -x /etc/uci-defaults/ddns_godaddy.com-v1 ] && \
			/etc/uci-defaults/ddns_godaddy.com-v1 && \
				rm -f /etc/uci-defaults/ddns_godaddy.com-v1 >/dev/null 2>&1
		/etc/init.d/ddns enabled \
			&& /etc/init.d/ddns start >/dev/null 2>&1
	}
	exit 0	# suppress errors
endef
define Package/ddns-scripts_godaddy.com-v1/prerm
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop				>/dev/null 2>&1
	# remove services file entries
	/bin/sed -i '/godaddy\.com-v1/d' $${IPKG_INSTROOT}/etc/ddns/services		>/dev/null 2>&1
	/bin/sed -i '/godaddy\.com-v1/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6	>/dev/null 2>&1
	exit 0	# suppress errors
endef

###### *************************************************************************
define Package/ddns-scripts_no-ip_com/preinst
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0	# suppress errors
endef
define Package/ddns-scripts_no-ip_com/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ddns.defaults $(1)/etc/uci-defaults/ddns_no-ip_com
	$(INSTALL_DIR) $(1)/usr/lib/ddns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/update_no-ip_com.sh $(1)/usr/lib/ddns
endef
define Package/ddns-scripts_no-ip_com/postinst
	#!/bin/sh
	# remove old services file entries
	/bin/sed -i '/no-ip\.com/d' $${IPKG_INSTROOT}/etc/ddns/services	>/dev/null 2>&1
	# and create new
	printf "%s\\t%s\\n" '"no-ip.com"' '"update_no-ip_com.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services
	# on real system restart service if enabled
	[ -z "$${IPKG_INSTROOT}" ] && {
		[ -x /etc/uci-defaults/ddns_no-ip_com ] && \
			/etc/uci-defaults/ddns_no-ip_com && \
				rm -f /etc/uci-defaults/ddns_no-ip_com >/dev/null 2>&1
		/etc/init.d/ddns enabled && \
			/etc/init.d/ddns start >/dev/null 2>&1
	}
	exit 0	# suppress errors
endef
define Package/ddns-scripts_no-ip_com/prerm
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop		>/dev/null 2>&1
	# remove services file entries
	/bin/sed -i '/no-ip\.com/d' $${IPKG_INSTROOT}/etc/ddns/services	>/dev/null 2>&1
	exit 0	# suppress errors
endef

###### *************************************************************************
define Package/ddns-scripts_nsupdate/preinst
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0	# suppress errors
endef
define Package/ddns-scripts_nsupdate/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ddns.defaults $(1)/etc/uci-defaults/ddns_nsupdate
	$(INSTALL_DIR) $(1)/usr/lib/ddns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/update_nsupdate.sh $(1)/usr/lib/ddns
endef
define Package/ddns-scripts_nsupdate/postinst
	#!/bin/sh
	# remove old services file entries
	/bin/sed -i '/bind-nsupdate/d' $${IPKG_INSTROOT}/etc/ddns/services	>/dev/null 2>&1
	/bin/sed -i '/bind-nsupdate/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6	>/dev/null 2>&1
	# and create new
	printf "%s\\t%s\\n" '"bind-nsupdate"' '"update_nsupdate.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services
	printf "%s\\t%s\\n" '"bind-nsupdate"' '"update_nsupdate.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services_ipv6
	# on real system restart service if enabled
	[ -z "$${IPKG_INSTROOT}" ] && {
		[ -x /etc/uci-defaults/ddns_nsupdate ] && \
			/etc/uci-defaults/ddns_nsupdate && \
				rm -f /etc/uci-defaults/ddns_nsupdate >/dev/null 2>&1
		/etc/init.d/ddns enabled && \
			/etc/init.d/ddns start >/dev/null 2>&1
	}
	exit 0	# suppress errors
endef
define Package/ddns-scripts_nsupdate/prerm
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop			>/dev/null 2>&1
	# remove services file entries
	/bin/sed -i '/bind-nsupdate/d' $${IPKG_INSTROOT}/etc/ddns/services	>/dev/null 2>&1
	/bin/sed -i '/bind-nsupdate/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6	>/dev/null 2>&1
	exit 0	# suppress errors
endef

###### *************************************************************************
define Package/ddns-scripts_route53-v1/preinst
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0	# suppress errors
endef
define Package/ddns-scripts_route53-v1/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/ddns.defaults $(1)/etc/uci-defaults/ddns_route53-v1
	$(INSTALL_DIR) $(1)/usr/lib/ddns
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/update_route53_v1.sh $(1)/usr/lib/ddns
endef
define Package/ddns-scripts_route53-v1/postinst
	#!/bin/sh
	# remove old services file entries
	/bin/sed -i '/route53-v1/d' $${IPKG_INSTROOT}/etc/ddns/services		>/dev/null 2>&1
	/bin/sed -i '/route53-v1/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6	>/dev/null 2>&1
	# and create new
	printf "%s\\t%s\\n" '"route53-v1"' '"update_route53_v1.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services
	printf "%s\\t%s\\n" '"route53-v1"' '"update_route53_v1.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services_ipv6
	# on real system restart service if enabled
	[ -z "$${IPKG_INSTROOT}" ] && {
		[ -x /etc/uci-defaults/ddns_route53-v1 ] && \
			/etc/uci-defaults/ddns_route53-v1 && \
				rm -f /etc/uci-defaults/route53.com-v1 >/dev/null 2>&1
		/etc/init.d/ddns enabled \
			&& /etc/init.d/ddns start >/dev/null 2>&1
	}
	exit 0	# suppress errors
endef
define Package/ddns-scripts_route53-v1/prerm
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop				>/dev/null 2>&1
	# remove services file entries
	/bin/sed -i 'route53-v1/d' $${IPKG_INSTROOT}/etc/ddns/services		>/dev/null 2>&1
	/bin/sed -i 'route53-v1/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6	>/dev/null 2>&1
	exit 0	# suppress errors
endef

###### *************************************************************************
$(eval $(call BuildPackage,ddns-scripts))
$(eval $(call BuildPackage,ddns-scripts_cloudflare.com-v4))
$(eval $(call BuildPackage,ddns-scripts_freedns_42_pl))
$(eval $(call BuildPackage,ddns-scripts_godaddy.com-v1))
$(eval $(call BuildPackage,ddns-scripts_no-ip_com))
$(eval $(call BuildPackage,ddns-scripts_nsupdate))
$(eval $(call BuildPackage,ddns-scripts_route53-v1))
