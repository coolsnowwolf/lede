#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=heimdal
PKG_VERSION:=1.5.3
PKG_RELEASE:=1
PKG_INSTALL:=1

PKG_BUILD_DEPENDS:=heimdal/host

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.h5l.org/dist/src/
PKG_MD5SUM:=30b379e3de12f332fbd201131f02ffca
#PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--disable-shared \
	--enable-static \
	--without-openldap \
	--without-capng \
	--without-sqlite3 \
	--without-libintl \
	--without-openssl \
	--without-berkeley-db \
	--without-readline \
	--without-libedit \
	--without-hesiod \
	--without-x \
	--disable-heimdal-documentation

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)/include $(HOST_MAKE_FLAGS)
	$(MAKE) -C $(HOST_BUILD_DIR)/lib/roken $(HOST_MAKE_FLAGS)
	$(MAKE) -C $(HOST_BUILD_DIR)/lib/vers $(HOST_MAKE_FLAGS)
	$(MAKE) -C $(HOST_BUILD_DIR)/lib/com_err $(HOST_MAKE_FLAGS)
	$(MAKE) -C $(HOST_BUILD_DIR)/lib/asn1 $(HOST_MAKE_FLAGS)
	$(MAKE) -C $(HOST_BUILD_DIR)/lib/libedit $(HOST_MAKE_FLAGS)
	$(MAKE) -C $(HOST_BUILD_DIR)/lib/sl $(HOST_MAKE_FLAGS)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR)/host/bin
	$(CP) $(HOST_BUILD_DIR)/lib/com_err/compile_et $(STAGING_DIR)/host/bin
	$(CP) $(HOST_BUILD_DIR)/lib/asn1/asn1_compile $(STAGING_DIR)/host/bin
	$(CP) $(HOST_BUILD_DIR)/lib/sl/slc $(STAGING_DIR)/host/bin
endef

CONFIGURE_ARGS += \
	--disable-pk-init \
	--disable-kx509 \
	--disable-sqlite-cache \
	--enable-pthread-support \
	--disable-afs-support \
	--disable-ndbm-db \
	--with-cross-tools=$(STAGING_DIR_HOST)/bin \
	--disable-heimdal-documentation \
	--enable-developer \
	--with-sqlite3=yes \
	--with-sqlite3-include=$(STAGING_DIR)/usr/include \
	--with-sqlite3-lib=$(STAGING_DIR)/usr/lib \
	--with-openssl=yes \
	--with-openssl-include=$(STAGING_DIR)/usr/include \
	--with-openssl-lib=$(STAGING_DIR)/usr/lib \
	--without-x \
	--disable-afs-support \
	--disable-ndbm-db \
	--disable-mmap

EXTRA_CFLAGS += \
	-Wno-deprecated-declarations \
	-Wno-unused-variable \
	-Wno-unused-function \
	-Wno-maybe-uninitialized \
	-Wno-return-type \
	-Wno-enum-compare

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) files/roken-h-process.pl $(PKG_BUILD_DIR)/cf
endef

define Build/Compile
endef


define Package/heimdal/Default
	SECTION:=network
	CATEGORY:=Network
	SUBMENU:=Kerberos
	TITLE:=Heimdal Kerberos 5
	URL:=http://www.h5l.org
endef

define Package/heimdal-libs
  $(call Package/heimdal/Default)
  TITLE+= (Libraries)
  DEPENDS:=+libsqlite3 +libreadline +libncurses +libopenssl
endef

define Package/heimdal-server
  $(call Package/heimdal/Default)
  TITLE+= (Server)
  DEPENDS:=+heimdal-libs
endef

define Package/heimdal-client
  $(call Package/heimdal/Default)
  TITLE+= (Client)
  DEPENDS:=+heimdal-libs
endef

define Package/heimdal-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
endef

define Package/heimdal-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kdestroy $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kinit $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kcc $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/klist $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kpasswd $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/su $(1)/usr/bin/ksu
endef

define Package/heimdal-server/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kadmin $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ktutil $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kstash $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/kdc $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/kadmind $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/kpasswdd $(1)/usr/sbin
endef

#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/include/heimdal
#	$(CP) $(PKG_BUILD_DIR)/include/heimdal/heimdal.h $(TOOLCHAIN_DIR)/include/sys
#	$(CP) $(PKG_BUILD_DIR)/include/heimdal/libheimdal.h $(1)/usr/include/heimdal
#	$(INSTALL_DIR) $(1)/usr/lib/
#	$(CP) $(PKG_BUILD_DIR)/libheimdal/.libs/libheimdal.{a,so*} $(1)/usr/lib/
#endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,heimdal-libs))
$(eval $(call BuildPackage,heimdal-client))
$(eval $(call BuildPackage,heimdal-server))
