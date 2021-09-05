include $(TOPDIR)/rules.mk

PKG_NAME:=syslog-ng
PKG_VERSION:=3.32.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=LGPL-2.1-or-later GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING LGPL.txt GPL.txt
PKG_CPE_ID:=cpe:/a:balabit:syslog-ng

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/syslog-ng/syslog-ng/releases/download/$(PKG_NAME)-$(PKG_VERSION)/
PKG_HASH:=19e3b76552d82c71c04230526925402c9f05fa5e6ea19b912b061850929b712d

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_IPV6 \

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/syslog-ng
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=A powerful syslog daemon
  URL:=https://www.syslog-ng.com/products/open-source-log-management/
  DEPENDS:=+libpcre +glib2 +libopenssl +libpthread +librt +zlib +libdbi +libjson-c +libcurl +libuuid
endef

define Package/syslog-ng/description
  syslog-ng reads and logs messages to the system console, log
  files, other machines and/or users as specified by its
  configuration file.
endef

define Package/syslog-ng/conffiles
/etc/syslog-ng.conf
/etc/syslog-ng.d/
/etc/scl.conf
endef

define Build/Configure
	$(SED) 's,-I/usr/include,,' $(PKG_BUILD_DIR)/configure
	$(Build/Configure/Default)
endef

CONFIGURE_ARGS +=  \
	--disable-afsnmp \
	$(call autoconf_bool,CONFIG_IPV6,ipv6) \
	--disable-tcp-wrapper \
	--disable-spoof-source \
	--disable-sql \
	--disable-linux-caps \
	--with-jsonc=system \
	--enable-json=yes \
	--enable-http=yes \
	--disable-smtp \
	--disable-redis \
	--disable-dependency-tracking \
	--disable-python \
	--disable-geoip2 \
	--disable-java \
	--disable-java-modules \
	--with-librabbitmq-client=no \
	--with-mongoc=no

CONFIGURE_VARS += \
	LIBDBI_CFLAGS="-I$(STAGING_DIR)/usr/include"

define Package/syslog-ng/install
	cd $(PKG_BUILD_DIR); make DESTDIR=$(1) install

	$(call libtool_remove_files,$(1)) # This removes .la files in folder (including subfolders) /usr/lib
	rm -rf $(1)/usr/lib/pkgconfig \
	$(1)/usr/lib/*.a \
	$(1)/usr/include \
	$(1)/var

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/syslog-ng.init $(1)/etc/init.d/syslog-ng
	$(INSTALL_DIR) $(1)/etc/syslog-ng.d
	$(INSTALL_DATA) ./files/syslog-ng.conf $(1)/etc
	touch $(1)/etc/syslog-ng.d/.keep

	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/logread $(1)/sbin

	$(INSTALL_DIR) $(1)/usr/share/syslog-ng/include/
	$(CP) -r ./files/scl $(1)/usr/share/syslog-ng/include/
endef

define Package/syslog-ng/postinst
#!/bin/sh

[ -n "$$IPKG_INSTROOT" ] || {
/etc/init.d/syslog-ng enable
/etc/init.d/syslog-ng restart
}
endef

define Package/syslog-ng/prerm
#!/bin/sh

[ -n "$$IPKG_INSTROOT" ] || {
/etc/init.d/syslog-ng disable
/etc/init.d/syslog-ng stop
}
endef

$(eval $(call BuildPackage,syslog-ng))
