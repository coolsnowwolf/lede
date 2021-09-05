#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tcp_wrappers
PKG_VERSION:=7.6
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=ftp://ftp.porcupine.org/pub/security
PKG_HASH:=9543d7adedf78a6de0b221ccbbd1952e08b5138717f4ade814039bb489a4315d

PKG_LICENSE:=BSD
PKG_LICENE_FILES:=DISCLAIMER

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)_$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/libwrap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Security wrapper library for TCP services
  URL:=ftp://ftp.porcupine.org/pub/security/index.html
  MAINTAINER:=Peter Wagner <tripolar@gmx.at>
endef

TARGET_CFLAGS += $(FPIC) -Wall

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		OPT_CFLAGS="$(TARGET_CFLAGS)" \
		$(TARGET_EXTRA_LIBS) \
		NETGROUP= \
		VSYSLOG= \
		BUGS= \
		EXTRA_CFLAGS="-DSYS_ERRLIST_DEFINED -DHAVE_STRERROR -DHAVE_WEAKSYMS -D_REENTRANT -DINET6=1 \
			-Dss_family=__ss_family -Dss_len=__ss_len" \
		FACILITY=LOG_DAEMON \
		SEVERITY=LOG_INFO \
		REAL_DAEMON_DIR=/usr/sbin \
		STYLE="-DPROCESS_OPTIONS" \
		tidy all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/tcpd.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libwrap.a $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/shared/libwrap.so* $(1)/usr/lib/
endef

define Package/libwrap/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/shared/libwrap.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libwrap))
