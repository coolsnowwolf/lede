include $(TOPDIR)/rules.mk

PKG_NAME:=ddns-scripts_aliyun
PKG_VERSION:=1.0.3
PKG_RELEASE:=1

PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Sense <sensec@gmail.com>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ddns-scripts_aliyun
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=DDNS extension for AliYun.com
  PKGARCH:=all
  DEPENDS:=+ddns-scripts +wget-ssl +openssl-util
endef

define Package/ddns-scripts_aliyun/description
  Dynamic DNS Client scripts extension for AliYun.com
endef

define Build/Configure
endef

define Build/Compile
	$(CP) ./*.sh $(PKG_BUILD_DIR)
endef

define Package/ddns-scripts_aliyun/preinst
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0 # suppress errors
endef

define Package/ddns-scripts_aliyun/install
	$(INSTALL_DIR) $(1)/usr/lib/ddns $(1)/usr/share/ddns/default
	$(INSTALL_BIN) $(CURDIR)/update_aliyun_com.sh $(1)/usr/lib/ddns
	$(INSTALL_DATA) $(CURDIR)/aliyun.com.json $(1)/usr/share/ddns/default
endef

define Package/ddns-scripts_aliyun/postinst
	#!/bin/sh
	# remove old services file entries
	/bin/sed -i '/aliyun\.com/d' $${IPKG_INSTROOT}/etc/ddns/services >/dev/null 2>&1
	/bin/sed -i '/aliyun\.com/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6 >/dev/null 2>&1
	# and create new
	printf "%s\\t\\t%s\\n" '"aliyun.com"' '"update_aliyun_com.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services
	printf "%s\\t\\t%s\\n" '"aliyun.com"' '"update_aliyun_com.sh"' >> $${IPKG_INSTROOT}/etc/ddns/services_ipv6
	# on real system restart service if enabled
	[ -z "$${IPKG_INSTROOT}" ] && {
		/etc/init.d/ddns enabled && \
			/etc/init.d/ddns start >/dev/null 2>&1
	}
	exit 0 # suppress errors
endef

define Package/ddns-scripts_aliyun/prerm
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	# remove services file entries
	/bin/sed -i '/aliyun\.com/d' $${IPKG_INSTROOT}/etc/ddns/services >/dev/null 2>&1
	/bin/sed -i '/aliyun\.com/d' $${IPKG_INSTROOT}/etc/ddns/services_ipv6 >/dev/null 2>&1
	exit 0 # suppress errors
endef

$(eval $(call BuildPackage,ddns-scripts_aliyun))
