#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

#TODO
#
# * uClibcpp not supported, performance +100x slower. libstdcpp is being used. https://bugs.busybox.net/show_bug.cgi?id=2545
# * liblua is still not recognized/blindly accepted. Impossible to compile with support
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nmap
PKG_VERSION:=7.91
PKG_RELEASE:=$(AUTORELEASE)
PKG_MAINTAINER:=Nuno Goncalves <nunojpg@gmail.com>

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://nmap.org/dist/
PKG_HASH:=18cc4b5070511c51eb243cdd2b0b30ff9b2c4dc4544c6312f75ce3a67a593300
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:nmap:nmap

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk

NMAP_DEPENDS:=+libpcap +libstdcpp +zlib +libpcre
NCAT_DEPENDS:=+libpcap
NPING_DEPENDS:=+libpcap +libpthread +libstdcpp

define Package/nmap/default
  SUBMENU:=NMAP Suite
  SECTION:=net
  CATEGORY:=Network
  URL:=https://nmap.org/
endef

define Package/nmap
$(call Package/nmap/default)
  DEPENDS:=$(NMAP_DEPENDS)
  VARIANT:=nossl
  TITLE:=Utility for network exploration or security auditing
endef

define Package/nmap-ssl
$(call Package/nmap/default)
  DEPENDS:=$(NMAP_DEPENDS) +libopenssl
  VARIANT:=ssl
  TITLE:=Nmap (with OpenSSL support)
endef

define Package/nmap-full
$(call Package/nmap/default)
  DEPENDS:=$(NMAP_DEPENDS) +libopenssl +liblua5.3 +libssh2
  VARIANT:=full
  TITLE:=Nmap (with OpenSSL and scripting support)
endef

define Package/ncat
$(call Package/nmap/default)
  DEPENDS:=$(NCAT_DEPENDS)
  VARIANT:=nossl
  TITLE:=Much-improved reimplementation of Netcat
endef

define Package/ncat-ssl
$(call Package/nmap/default)
  DEPENDS:=$(NCAT_DEPENDS) +libopenssl
  VARIANT:=ssl
  TITLE:=Ncat (with OpenSSL support)
endef

define Package/ncat-full
$(call Package/nmap/default)
  DEPENDS:=$(NCAT_DEPENDS) +libopenssl +liblua5.3
  VARIANT:=full
  TITLE:=Ncat (with OpenSSL and scripting support)
endef

define Package/nping
$(call Package/nmap/default)
  DEPENDS:=$(NPING_DEPENDS)
  VARIANT:=nossl
  TITLE:=Network packet generation tool / ping utility
endef

define Package/nping-ssl
$(call Package/nmap/default)
  DEPENDS:=$(NPING_DEPENDS) +libopenssl
  VARIANT:=ssl
  TITLE:=Nping (with OpenSSL support)
endef

define Package/ndiff
$(call Package/nmap/default)
  DEPENDS:=+python3-light +python3-xml
  TITLE:=Utility to compare the results of Nmap scans
endef

CONFIGURE_ARGS += \
	--with-libdnet=included \
	--with-liblinear=included \
	--with-libpcap="$(STAGING_DIR)/usr" \
	--with-libpcre="$(STAGING_DIR)/usr" \
	--with-libz="$(STAGING_DIR)/usr" \
	--with-ncat \
	--without-localdirs \
	--without-ndiff \
	--without-zenmap
	# --with-libnbase=included
	# --with-libnsock=included

ifeq ($(BUILD_VARIANT),full)
  CONFIGURE_ARGS += \
	--with-liblua="$(STAGING_DIR)/usr" \
	--with-libssh2="$(STAGING_DIR)/usr" \
	--with-openssl="$(STAGING_DIR)/usr" \
	--without-nping

else ifeq ($(BUILD_VARIANT),ssl)
  CONFIGURE_ARGS += \
	--with-nping \
	--with-openssl="$(STAGING_DIR)/usr" \
	--without-liblua \
	--without-libssh2

else  # nossl
  CONFIGURE_ARGS += \
	--with-nping \
	--without-liblua \
	--without-libssh2 \
	--without-openssl
endif

CONFIGURE_VARS += \
	ac_cv_dnet_bsd_bpf=no

PYTHON3_PKG_SETUP_DIR:=ndiff
PYTHON3_PKG_SETUP_ARGS:=

define Build/Compile
	$(call Build/Compile/Default,)
	$(call Py3Build/Compile)
endef

define Package/nmap/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nmap $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/nmap
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/nmap/nmap* $(1)/usr/share/nmap/
endef

Package/nmap-ssl/install=$(Package/nmap/install)

define Package/nmap-full/install
	$(call Package/nmap/install,$(1))
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/nmap/nse_main.lua $(1)/usr/share/nmap/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/nmap/{nselib,scripts} $(1)/usr/share/nmap/
endef

define Package/ncat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ncat $(1)/usr/bin/
endef

define Package/ncat-ssl/install
	$(call Package/ncat/install,$(1))
	$(INSTALL_DIR) $(1)/usr/share/ncat
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/ncat/ca-bundle.crt $(1)/usr/share/ncat/
endef

Package/ncat-full/install=$(Package/ncat-ssl/install)

define Package/nping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nping $(1)/usr/bin/
endef

Package/nping-ssl/install=$(Package/nping/install)

define Py3Package/ndiff/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ndiff $(1)/usr/bin/
endef

$(eval $(call BuildPackage,nmap))
$(eval $(call BuildPackage,nmap-ssl))
$(eval $(call BuildPackage,nmap-full))
$(eval $(call BuildPackage,ncat))
$(eval $(call BuildPackage,ncat-ssl))
$(eval $(call BuildPackage,ncat-full))
$(eval $(call BuildPackage,nping))
$(eval $(call BuildPackage,nping-ssl))

$(eval $(call Py3Package,ndiff))
$(eval $(call BuildPackage,ndiff))
$(eval $(call BuildPackage,ndiff-src))
