#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=apache
PKG_VERSION:=2.4.46
PKG_RELEASE:=2
PKG_SOURCE_NAME:=httpd

PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@APACHE/httpd/
PKG_HASH:=740eddf6e1c641992b22359cabc66e6325868c3c5e2e3f98faf349b61ecf41ea

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Thomas Heil <heil@terminal-consulting.de>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_CPE_ID:=cpe:/a:apache:http_server

PKG_BUILD_DEPENDS:=openssl

PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_apache-mod-deflate \
	CONFIG_PACKAGE_apache-mod-http2 \
	CONFIG_PACKAGE_apache-mod-ldap \
	CONFIG_PACKAGE_apache-mod-lua \
	CONFIG_PACKAGE_apache-mod-md \
	CONFIG_PACKAGE_apache-mod-proxy \
	CONFIG_PACKAGE_apache-mod-proxy-html \
	CONFIG_PACKAGE_apache-mod-session-crypto \
	CONFIG_PACKAGE_apache-mod-suexec \
	CONFIG_PACKAGE_apache-mod-webdav \
	CONFIG_PACKAGE_apache-suexec

PKG_FIXUP:=autoreconf

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
# without nls.mk mod_xml2enc might not find the iconv headers
include $(INCLUDE_DIR)/nls.mk

define Package/apache/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=The Apache Web Server
  URL:=https://httpd.apache.org/
endef

define Package/apache/Default/description
The Apache HTTP Server Project is a collaborative software development
effort aimed at creating a robust, commercial-grade, featureful, and
freely-available source code implementation of an HTTP (Web) server.
endef

define Package/apache
$(call Package/apache/Default)
  USERID:=apache=377:apache=377
  DEPENDS:=+libapr +libaprutil +libpcre
endef

define Package/apache/description
$(call Package/apache/Default/description)

This package contains the Apache web server and utility programs.

endef

define Package/apache/conffiles
/etc/apache2/apache2.conf
/etc/apache2/extra/httpd-autoindex.conf
/etc/apache2/extra/httpd-dav.conf
/etc/apache2/extra/httpd-default.conf
/etc/apache2/extra/httpd-info.conf
/etc/apache2/extra/httpd-languages.conf
/etc/apache2/extra/httpd-manual.conf
/etc/apache2/extra/httpd-mpm.conf
/etc/apache2/extra/httpd-multilang-errordoc.conf
/etc/apache2/extra/httpd-ssl.conf
/etc/apache2/extra/httpd-userdir.conf
/etc/apache2/extra/httpd-vhosts.conf
/etc/apache2/extra/proxy-html.conf
/etc/apache2/magic
/etc/apache2/mime.types
/etc/init.d/apache2
endef

define Package/apache-ab
$(call Package/apache/Default)
  TITLE:=Apache benchmark utility
  DEPENDS:=apache +libopenssl
endef

define Package/apache-ab/description
$(call Package/apache/Default/description)

Apache server benchmarking utility.

endef

define Package/apache-error
$(call Package/apache/Default)
  TITLE:=Error documents
  DEPENDS:=apache
endef

define Package/apache-error/description
$(call Package/apache/Default/description)

Apache multi language custom error documents.

endef

define Package/apache-icons
$(call Package/apache/Default)
  TITLE:=Icons from Apache
  DEPENDS:=apache
endef

define Package/apache-icons/description
$(call Package/apache/Default/description)

This package contains the icons from Apache.

endef

define Package/apache-suexec
$(call Package/apache/Default)
  TITLE:=Apache suEXEC
  DEPENDS:=apache
  # Directory "/usr/lib/apache2/suexec_dir" is installed with mode 0750
  # and is only accessible by root and the group apache. This way apache
  # can access the SUID binary "suexec" contained within while others
  # cannot.
  FILE_MODES:=/usr/lib/apache2/suexec_dir:root:apache:0750
endef

define Package/apache-suexec/description
$(call Package/apache/Default/description)

This package contains the suEXEC utility from Apache.

endef

define Package/apache-utils
$(call Package/apache/Default)
  TITLE:=Apache utilities
  DEPENDS:=apache
endef

define Package/apache-utils/description
$(call Package/apache/Default/description)

Apache utility programs for webservers.

endef

define Package/apache/install/mod
	$(INSTALL_DIR) $(1)/usr/lib/apache2
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/apache2/mod_$(2).so \
					$(1)/usr/lib/apache2
endef

CONFIGURE_ARGS+= \
	--$(if $(CONFIG_PACKAGE_apache-mod-http2),en,dis)able-http2 \
	--$(if $(CONFIG_PACKAGE_apache-mod-lua),en,dis)able-lua \
	--$(if $(CONFIG_PACKAGE_apache-mod-md),en,dis)able-md \
	--$(if $(CONFIG_PACKAGE_apache-mod-proxy),en,dis)able-proxy \
	--$(if $(CONFIG_PACKAGE_apache-mod-session-crypto),en,dis)able-session-crypto \
	--$(if $(CONFIG_PACKAGE_apache-mod-ssl),en,dis)able-ssl \
	--datadir=/usr/share/apache2 \
	--disable-imagemap \
	--disable-luajit \
	--enable-authn-alias \
	--enable-authn-anon \
	--enable-cache \
	--enable-cgi \
	--enable-cgid \
	--enable-dbd \
	--enable-disk-cache \
	--enable-exception-hook \
	--enable-file-cache \
	--enable-layout=OpenWrt \
	--enable-mem-cache \
	--enable-mods-shared=all \
	--enable-mpms-shared=all \
	--enable-so \
	--libexecdir=/usr/lib/apache2 \
	--sysconfdir=/etc/apache2 \
	--with-apr-util="$(STAGING_DIR)/usr/bin/apu-1-config" \
	--with-apr="$(STAGING_DIR)/usr/bin/apr-1-config" \
	--with-mpm=prefork \
	--with-pcre="$(STAGING_DIR)/usr/bin/pcre-config" \
	--with-program-name=apache2 \
	--with-ssl

ifneq ($(CONFIG_PACKAGE_apache-mod-deflate),)
CONFIGURE_ARGS+= \
	--enable-deflate \
	--with-z="$(STAGING_DIR)/usr"
else
CONFIGURE_ARGS+= \
	--disable-deflate \
	--without-z
endif

ifneq ($(CONFIG_PACKAGE_apache-mod-ldap),)
CONFIGURE_ARGS+= \
	--enable-authnz-ldap \
	--enable-ldap
else
CONFIGURE_ARGS+= \
	--disable-authnz-ldap \
	--disable-ldap
endif

ifneq ($(CONFIG_PACKAGE_apache-mod-proxy),)
CONFIGURE_ARGS+= \
	--enable-proxy \
	--enable-proxy-connect \
	--enable-proxy-ftp \
	--enable-proxy-http
else
CONFIGURE_ARGS+= \
	--disable-proxy
endif

ifneq ($(CONFIG_PACKAGE_apache-mod-proxy-html),)
CONFIGURE_ARGS+= \
	--enable-proxy-html \
	--enable-xml2enc \
	--with-libxml2="$(STAGING_DIR)/usr"
else
CONFIGURE_ARGS+= \
	--disable-proxy-html \
	--disable-xml2enc
endif

ifneq ($(CONFIG_PACKAGE_apache-mod-suexec)$(CONFIG_PACKAGE_apache-suexec),)
CONFIGURE_ARGS+= \
	--enable-suexec \
	--with-suexec-bin=/usr/lib/apache2/suexec_dir/suexec \
	--with-suexec-caller=apache \
	--with-suexec-logfile=/var/log/apache2/suexec.log
else
CONFIGURE_ARGS+= \
	--disable-suexec
endif

ifneq ($(CONFIG_PACKAGE_apache-mod-webdav),)
CONFIGURE_ARGS+= \
	--enable-dav \
	--enable-dav-fs \
	--enable-dav-lock
else
CONFIGURE_ARGS+= \
	--disable-dav
endif

CONFIGURE_VARS += \
	ac_cv_gettid=yes \
	ap_cv_void_ptr_lt_long=no

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/apxs $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/include/apache2
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/apache2/* \
					$(1)/usr/include/apache2
	$(INSTALL_DIR) $(1)/usr/lib/apache2
	$(INSTALL_DIR) $(1)/usr/share/apache2/build
	$(CP) $(PKG_INSTALL_DIR)/usr/share/apache2/build/* \
				$(1)/usr/share/apache2/build
	$(SED) 's%^prefix =.*%prefix = $(STAGING_DIR)/usr%' \
		$(1)/usr/share/apache2/build/config_vars.mk
endef

define Package/apache/install
	$(INSTALL_DIR) $(1)/etc/apache2/extra
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/apache2/extra/* \
					$(1)/etc/apache2/extra
	$(SED) '/^LoadModule session_crypto_module/s/^/#/' \
		$(PKG_INSTALL_DIR)/etc/apache2/apache2.conf
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/etc/apache2/{apache2.conf,magic,mime.types} \
								$(1)/etc/apache2
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/apache2.init $(1)/etc/init.d/apache2
	$(INSTALL_DIR) $(1)/usr/lib/apache2
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/apache2/*.so \
					$(1)/usr/lib/apache2
	rm -f $(1)/usr/lib/apache2/mod_{*ldap,dav*,deflate,http2,lbmethod_*,lua,md,proxy*,proxy_html,session_crypto,ssl,suexec,xml2enc}.so
	$(INSTALL_DIR) $(1)/usr/share/apache2/{cgi-bin,htdocs}
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/apache2/cgi-bin/* \
					$(1)/usr/share/apache2/cgi-bin
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/apache2/htdocs/* \
					$(1)/usr/share/apache2/htdocs
	$(INSTALL_DIR) $(1)/usr/{,s}bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/{dbmmanage,htdbm,htdigest,htpasswd,httxt2dbm,logresolve} \
											$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{apachectl,apache2} \
							$(1)/usr/sbin
endef

define Package/apache-ab/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ab $(1)/usr/bin
endef

define Package/apache-error/install
	$(INSTALL_DIR) $(1)/usr/share/apache2/error
	$(CP) $(PKG_INSTALL_DIR)/usr/share/apache2/error/* \
				$(1)/usr/share/apache2/error
endef

define Package/apache-icons/install
	$(INSTALL_DIR) $(1)/usr/share/apache2
	$(CP) $(PKG_INSTALL_DIR)/usr/share/apache2/icons \
				$(1)/usr/share/apache2
endef

define Package/apache-suexec/install
	$(INSTALL_DIR) $(1)/usr/lib/apache2/suexec_dir
	$(INSTALL_SUID) $(PKG_INSTALL_DIR)/usr/sbin/suexec \
			$(1)/usr/lib/apache2/suexec_dir
endef

define Package/apache-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/{checkgid,envvars*,htcacheclean,rotatelogs} \
									$(1)/usr/sbin
endef

define Package/apache/Module
define Package/apache-mod-$(1)
$(call Package/apache/Default)
  TITLE:=$(2) module
  DEPENDS:=apache $(patsubst +%,+PACKAGE_apache-mod-$(1):%,$(4))
endef
define Package/apache-mod-$(1)/description
$(subst \n,$(newline),$(3))
endef
define Package/apache-mod-$(1)/install
$(foreach m,$(5),$(call Package/apache/install/mod,$$(1),$(m));)
endef
$$(eval $$(call BuildPackage,apache-mod-$(1)))
endef

$(eval $(call BuildPackage,apache))
$(eval $(call BuildPackage,apache-ab))
$(eval $(call BuildPackage,apache-error))
$(eval $(call BuildPackage,apache-icons))
$(eval $(call BuildPackage,apache-suexec))
$(eval $(call BuildPackage,apache-utils))
$(eval $(call Package/apache/Module,deflate,Deflate,Deflate support for the Apache HTTP server.,+zlib,deflate))
$(eval $(call Package/apache/Module,http2,HTTP2,HTTP/2 transport layer for the Apache HTTP Server.,+libnghttp2 +libopenssl,http2))
$(eval $(call Package/apache/Module,ldap,LDAP,LDAP authentication/authorization module for the Apache HTTP Server.,+libaprutil-ldap,authnz_ldap ldap))
$(eval $(call Package/apache/Module,lua,Lua,Lua support for the Apache HTTP server.,+liblua,lua))
$(eval $(call Package/apache/Module,md,Managed Domain handling,Managed Domain handling.,+libcurl +jansson +libopenssl,md))
$(eval $(call Package/apache/Module,proxy,Proxy,Proxy modules for the Apache HTTP Server.,,proxy proxy_ajp proxy_balancer proxy_connect proxy_express proxy_fcgi proxy_fdpass proxy_ftp proxy_hcheck proxy_http proxy_scgi proxy_uwsgi proxy_wstunnel lbmethod_byrequests lbmethod_heartbeat lbmethod_bytraffic lbmethod_bybusyness))
$(eval $(call Package/apache/Module,proxy-html,Proxy HTML,HTML and XML content filters for the Apache HTTP Server.,+apache-mod-proxy +libxml2,proxy_html xml2enc))
$(eval $(call Package/apache/Module,session-crypto,Session crypto,Session encryption support for the Apache HTTP Server.,+libaprutil-crypto-openssl,session_crypto))
$(eval $(call Package/apache/Module,ssl,SSL/TLS,SSL/TLS module for the Apache HTTP Server.,+libopenssl,ssl))
$(eval $(call Package/apache/Module,suexec,suEXEC,suEXEC module for the Apache HTTP Server.,+apache-suexec,suexec))
$(eval $(call Package/apache/Module,webdav,WebDAV,WebDAV support for the Apache HTTP Server.,,dav dav_fs dav_lock))
