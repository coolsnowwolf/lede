#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=privoxy
PKG_VERSION:=3.0.28
PKG_RELEASE:=1

PKG_SOURCE:=privoxy-$(PKG_VERSION)-stable-src.tar.gz
PKG_SOURCE_URL:=@SF/ijbswa
PKG_HASH:=b5d78cc036aaadb3b7cf860e9d598d7332af468926a26e2d56167f1cb6f2824a
PKG_BUILD_DIR:=$(BUILD_DIR)/privoxy-$(PKG_VERSION)-stable
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:privoxy:privoxy

PKG_MAINTAINER:=

PKG_CONFIG_DEPENDS:= \
  CONFIG_IPV6 \
  \
  CONFIG_PRIVOXY_disable-pthread \
  CONFIG_PRIVOXY_disable-dynamic-pcre \
  CONFIG_PRIVOXY_disable-zlib \
  \
  CONFIG_PRIVOXY_disable-toggle \
  CONFIG_PRIVOXY_disable-force \
  CONFIG_PRIVOXY_disable-fast-redirects \
  CONFIG_PRIVOXY_disable-stats \
  CONFIG_PRIVOXY_enable-ie-images \
  CONFIG_PRIVOXY_disable-image-blocking \
  CONFIG_PRIVOXY_disable-acl-support \
  CONFIG_PRIVOXY_disable-trust-files \
  CONFIG_PRIVOXY_disable-editor \
  CONFIG_PRIVOXY_enable-no-gifs \
  CONFIG_PRIVOXY_enable-graceful-termination \
  CONFIG_PRIVOXY_enable-extended-host-patterns \
  CONFIG_PRIVOXY_enable-external-filters \
  CONFIG_PRIVOXY_enable-accept-filter \
  CONFIG_PRIVOXY_enable-strptime-sanity-checks \
  CONFIG_PRIVOXY_enable-compression

include $(INCLUDE_DIR)/package.mk

define Package/privoxy
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=Privoxy WEB Proxy
  URL:=http://www.privoxy.org/
  USERID:=privoxy=8118:privoxy=8118
  MENU:=1
  DEPENDS:=+!PRIVOXY_no_pcre:libpcre +!PRIVOXY_no_pthread:libpthread +!PRIVOXY_no_zlib:zlib
endef

# shown in LuCI package description
define Package/privoxy/description
$(TITLE) - Homepage: www.privoxy.org
endef

# shown in make menuconfig <Help>
define Package/privoxy/config
	help
		Privoxy is a web proxy with advanced filtering capabilities for protecting
		privacy, modifying web page content, managing cookies, controlling access,
		and removing ads, banners, pop-ups and other obnoxious Internet junk. 
		Privoxy has a very flexible configuration and can be customized to suit
		individual needs and tastes. 
		Privoxy has application for both stand-alone systems and multi-user networks.
		Run as : $(USERID)
		Version: $(PKG_VERSION)-$(PKG_RELEASE)
		Home   : $(URL)

	source "$(SOURCE)/Config.in"
endef

CONFIGURE_ARGS += \
	--sysconfdir=/etc/privoxy \
	$(if $(CONFIG_IPV6),,--disable-ipv6-support) \
	\
	$(if $(CONFIG_PRIVOXY_disable-pthread),--disable-pthread) \
	$(if $(CONFIG_PRIVOXY_disable-dynamic-pcre),--disable-dynamic-pcre) \
	$(if $(CONFIG_PRIVOXY_disable-zlib),--disable-zlib) \
	\
	$(if $(CONFIG_PRIVOXY_disable-toggle),--disable-toggle) \
	$(if $(CONFIG_PRIVOXY_disable-force),--disable-force) \
	$(if $(CONFIG_PRIVOXY_disable-fast-redirects),--disable-fast-redirects) \
	$(if $(CONFIG_PRIVOXY_disable-stats),--disable-stats) \
	$(if $(CONFIG_PRIVOXY_enable-ie-images),--enable-ie-images) \
	$(if $(CONFIG_PRIVOXY_disable-image-blocking),--disable-image-blocking) \
	$(if $(CONFIG_PRIVOXY_disable-acl-support),--disable-acl-support) \
	$(if $(CONFIG_PRIVOXY_disable-trust-files),--disable-trust-files) \
	$(if $(CONFIG_PRIVOXY_disable-editor),--disable-editor) \
	$(if $(CONFIG_PRIVOXY_enable-no-gifs),--enable-no-gifs) \
	$(if $(CONFIG_PRIVOXY_enable-graceful-termination),--enable-graceful-termination) \
	$(if $(CONFIG_PRIVOXY_enable-extended-host-patterns),--enable-extended-host-patterns) \
	$(if $(CONFIG_PRIVOXY_enable-external-filters),--enable-external-filters) \
	$(if $(CONFIG_PRIVOXY_enable-accept-filter),--enable-accept-filter) \
	$(if $(CONFIG_PRIVOXY_enable-strptime-sanity-checks),--enable-strptime-sanity-checks) \
	$(if $(CONFIG_PRIVOXY_enable-compression),--enable-compression)

# needed otherwise errors during compile
MAKE_FLAGS:=

define Package/privoxy/conffiles
/etc/config/privoxy
/etc/privoxy/user.action
/etc/privoxy/user.filter
/etc/privoxy/user.trust
endef

define Package/privoxy/preinst
	#!/bin/sh
	[ -n "$${IPKG_INSTROOT}" ] && exit 0	# if run within buildroot exit

	# stop service if PKG_UPGRADE
	[ "$${PKG_UPGRADE}" = "1" ] && /etc/init.d/privoxy stop >/dev/null 2>&1
		
	exit 0	# suppress errors from stop command
endef

define Package/privoxy/install
	if [ -f $(PKG_INSTALL_DIR)/etc/privoxy/trust ]; then \
		mv -f $(PKG_INSTALL_DIR)/etc/privoxy/trust $(PKG_INSTALL_DIR)/etc/privoxy/user.trust; \
	fi
	if [ -f $(PKG_INSTALL_DIR)/etc/privoxy/config ]; then \
		rm -f $(PKG_INSTALL_DIR)/etc/privoxy/config; \
	fi

	$(INSTALL_DIR) \
		$(1)/usr/sbin\
		$(1)/etc/privoxy/templates
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/privoxy $(1)/usr/sbin/
	find $(PKG_INSTALL_DIR)/etc/privoxy/templates -maxdepth 1 -type f \
		-exec $(INSTALL_DATA) -t $(1)/etc/privoxy/templates {} \;
	find $(PKG_INSTALL_DIR)/etc/privoxy -maxdepth 1 -type f \
		-exec install -m0664 -t $(1)/etc/privoxy {} \;

	$(INSTALL_DIR) \
		$(1)/lib/upgrade/keep.d \
		$(1)/etc/init.d \
		$(1)/etc/hotplug.d/iface \
		$(1)/etc/config
	$(INSTALL_DATA) ./files/privoxy.upgrade $(1)/lib/upgrade/keep.d/privoxy
	$(INSTALL_BIN)  ./files/privoxy.init $(1)/etc/init.d/privoxy
	$(INSTALL_BIN)  ./files/privoxy.hotplug $(1)/etc/hotplug.d/iface/95-privoxy
	$(INSTALL_CONF) ./files/privoxy.config $(1)/etc/config/privoxy
endef

define Package/privoxy/postinst
	#!/bin/sh
	grep -i privoxy $${IPKG_INSTROOT}/etc/services >/dev/null 2>&1 || \
		echo -e "privoxy\t8118" >> $${IPKG_INSTROOT}/etc/services
endef

$(eval $(call BuildPackage,privoxy))
