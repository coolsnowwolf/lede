#
# Copyright (C) 2011-2013 OpenWrt.org
# Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mosquitto
PKG_VERSION:=2.0.11
PKG_RELEASE:=1
PKG_LICENSE:=EPL-2.0
PKG_LICENSE_FILES:=LICENSE.txt
PKG_CPE_ID:=cpe:/a:eclipse:mosquitto

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://mosquitto.org/files/source/
PKG_HASH:=7b36a7198bce85cf31b132f5c6ee36dcf5dadf86fb768501eb1e11ce95d4f78a

include $(INCLUDE_DIR)/package.mk

define Package/mosquitto/default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=mosquitto - an MQTT message broker
  URL:=http://www.mosquitto.org/
  MAINTAINER:=Karl Palsson <karlp@etactica.com>
  DEPENDS:= +librt
  USERID:=mosquitto=200:mosquitto=200
endef

define Package/mosquitto-ssl
    $(call Package/mosquitto/default)
    TITLE+= (with SSL support)
    DEPENDS+= +libopenssl +MOSQUITTO_LWS:libwebsockets-openssl +MOSQUITTO_DYNAMIC_SECURITY:cJSON
    VARIANT:=ssl
    PROVIDES:=mosquitto
endef

define Package/mosquitto-nossl
    $(call Package/mosquitto/default)
    TITLE+= (without SSL support)
    VARIANT:=nossl
    PROVIDES:=mosquitto
endef

define Package/mosquitto-ssl/config
	source "$(SOURCE)/Config.in"
endef

define Package/mosquitto/default/description
Mosquitto is an open source (BSD licensed) message broker that implements
the MQTT protocol version 3.1, 3.1.1 and 5. MQTT provides a lightweight
method of carrying out messaging using a publish/subscribe model.

This package also includes some basic support for configuring via UCI
endef

define Package/mosquitto-ssl/description
	$(call Package/mosquitto/default/description)
This package is built with SSL support.  TLS-PSK will be included (in both
the client and broker) if OpenSSL is built with TLS-PSK support.
endef

define Package/mosquitto-nossl/description
        $(call Package/mosquitto/default/description)
        This package is built WITHOUT SSL support.
endef


define Package/mosquitto-client/default
    $(Package/mosquitto/default)
    TITLE:= mosquitto - client tools
    DEPENDS+=+libcares +cJSON
    PROVIDES:=mosquitto-client
endef
define Package/mosquitto-client-ssl
    $(call Package/mosquitto-client/default)
    TITLE+= (With SSL support)
    DEPENDS+=+libmosquitto-ssl
    VARIANT:=ssl
endef
define Package/mosquitto-client-nossl
    $(call Package/mosquitto-client/default)
    TITLE+= (Without SSL support)
    DEPENDS+=+libmosquitto-nossl
    VARIANT:=nossl
endef

define Package/mosquitto-client/default/description
 Command line client tools for working with mosquitto.
This includes mosquitto_sub, mosquitto_pub, mosquitto_rr
and the mosquitto_ctrl tool.
endef

define Package/mosquitto-client-ssl/description
$(call Package/mosquitto-client/default/description)
        This package is built with SSL support
endef
define Package/mosquitto-client-nossl/description
$(call Package/mosquitto-client/default/description)
        This package is built without SSL support
endef

define Package/libmosquitto/default
    $(Package/mosquitto/default)
    SECTION:=libs
    CATEGORY:=Libraries
    DEPENDS:=+libpthread +librt +libcares
    TITLE:= mosquitto - client library
    PROVIDES:=libmosquitto
endef

define Package/libmosquitto-ssl
    $(call Package/libmosquitto/default)
    TITLE+= (With SSL Support)
    DEPENDS+= +libopenssl
    VARIANT=ssl
endef
define Package/libmosquitto-nossl
    $(call Package/libmosquitto/default)
    TITLE+= (Without SSL Support)
    VARIANT=nossl
endef

define Package/libmosquitto/default/description
 Library required for mosquitto's command line client tools, also for
use by any third party software that wants to communicate with _any_
MQTT server.

Should be useable for communicating with any MQTT v3.1/3.1.1/5 compatible
server, in addition to Mosquitto.
endef

define Package/libmosquitto-ssl/description
    $(call Package/libmosquitto/default/description)
    This package is built with SSL support
endef
define Package/libmosquitto-nossl/description
    $(call Package/libmosquitto/default/description)
    This package is built without SSL support
endef

define Package/libmosquittopp
    SECTION:=libs
    CATEGORY:=Libraries
    DEPENDS:=libmosquitto +libstdcpp
    TITLE:= mosquitto - client c++ library
endef

define Package/libmosquittopp/description
 C++ Library wrapper for libmosquitto.
endef

define Package/mosquitto-ssl/conffiles
/etc/mosquitto/mosquitto.conf
/etc/config/mosquitto
endef

Package/mosquitto-nossl/conffiles = $(Package/mosquitto-ssl/conffiles)

define Package/mosquitto/install/default
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mosquitto $(1)/usr/sbin/mosquitto
	$(INSTALL_DIR) $(1)/etc/mosquitto
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/mosquitto.conf $(1)/etc/mosquitto/mosquitto.conf
	$(CP) ./files/* $(1)/
endef

define Package/mosquitto-nossl/install
	$(call Package/mosquitto/install/default,$(1))
endef

define Package/mosquitto-ssl/install
	$(call Package/mosquitto/install/default,$(1))
ifeq ($(CONFIG_MOSQUITTO_PASSWD),y)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/apps/mosquitto_passwd/mosquitto_passwd $(1)/usr/bin
endif
ifeq ($(CONFIG_MOSQUITTO_DYNAMIC_SECURITY),y)
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/plugins/dynamic-security/mosquitto_dynamic_security.so $(1)/usr/lib
endif
endef

define Package/mosquitto-client-nossl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/client/mosquitto_pub $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/client/mosquitto_sub $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/client/mosquitto_rr $(1)/usr/bin
endef
define Package/mosquitto-client-ssl/install
	$(call Package/mosquitto-client-nossl/install,$(1))
ifeq ($(CONFIG_MOSQUITTO_CTRL),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/apps/mosquitto_ctrl/mosquitto_ctrl $(1)/usr/bin
endif
endef

# This installs files into ./staging_dir/. so that you can cross compile from the host
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/lib/cpp/mosquittopp.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libmosquitto.so.1 $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/lib/cpp/libmosquittopp.so.1 $(1)/usr/lib/
	$(LN) libmosquitto.so.1 $(1)/usr/lib/libmosquitto.so
	$(LN) libmosquittopp.so.1 $(1)/usr/lib/libmosquittopp.so
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/libmosquitto.pc.in $(1)/usr/lib/pkgconfig/libmosquitto.pc
	sed -i -e "s#@CMAKE_INSTALL_PREFIX@#/usr#" \
	       -e "s#@VERSION@#$(PKG_VERSION)#" \
	    $(1)/usr/lib/pkgconfig/libmosquitto.pc
	$(CP) $(PKG_BUILD_DIR)/libmosquittopp.pc.in $(1)/usr/lib/pkgconfig/libmosquittopp.pc
	sed -i -e "s#@CMAKE_INSTALL_PREFIX@#/usr#" \
	       -e "s#@VERSION@#$(PKG_VERSION)#" \
	    $(1)/usr/lib/pkgconfig/libmosquittopp.pc
endef

# This installs files on the target.  Compare with Build/InstallDev
define Package/libmosquitto-ssl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/libmosquitto.so.1 $(1)/usr/lib/
	$(LN) libmosquitto.so.1 $(1)/usr/lib/libmosquitto.so
endef
Package/libmosquitto-nossl/install = $(Package/libmosquitto-ssl/install)

define Package/libmosquittopp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/cpp/libmosquittopp.so.1 $(1)/usr/lib/
	$(LN) libmosquittopp.so.1 $(1)/usr/lib/libmosquittopp.so
endef

# Applies to all...
MAKE_FLAGS += WITH_DOCS=no UNAME=Linux
ifeq ($(BUILD_VARIANT),nossl)
	MAKE_FLAGS += WITH_TLS=no WITH_WEBSOCKETS=no
else
	MAKE_FLAGS += WITH_WEBSOCKETS=$(if $(CONFIG_MOSQUITTO_LWS),"yes","no")
	MAKE_FLAGS += WITH_TLS_PSK=$(if $(CONFIG_OPENSSL_WITH_PSK),"yes","no")
endif

$(eval $(call BuildPackage,mosquitto-ssl))
$(eval $(call BuildPackage,mosquitto-nossl))
$(eval $(call BuildPackage,libmosquitto-ssl))
$(eval $(call BuildPackage,libmosquitto-nossl))
$(eval $(call BuildPackage,libmosquittopp))
$(eval $(call BuildPackage,mosquitto-client-ssl))
$(eval $(call BuildPackage,mosquitto-client-nossl))
