#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rrdtool1
PKG_VERSION:=1.0.50
PKG_RELEASE:=3

PKG_BUILD_DIR:=$(BUILD_DIR)/rrdtool-$(PKG_VERSION)
PKG_SOURCE:=rrdtool-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
	http://oss.oetiker.ch/rrdtool/pub/rrdtool-1.0.x \
	http://www.fastmirrors.org/rrdtool/rrdtool-1.0.x \
	ftp://ftp.sunet.se/pub/network/monitoring/rrdtool/rrdtool-1.0.x
PKG_HASH:=42aa7c213dedbd95d33ca84d92f4187880f7e96062c6a3fb05bfb16f77ba2a91
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_FIXUP:=autoreconf
PKG_CHECK_FORMAT_SECURITY:=0

include $(INCLUDE_DIR)/package.mk

define Package/rrdtool1/Default
  TITLE:=Round Robin Database (RRD)
  URL:=http://oss.oetiker.ch/rrdtool/
endef

define Package/rrdtool1/description/Default
 RRD is the Acronym for Round Robin Database. RRD is a system to store and
 display time-series data (i.e. network bandwidth, machine-room temperature,
 server load average). It stores the data in a very compact way that will
 not expand over time, and it presents useful graphs by processing the data
 to enforce a certain data density. It can be used either via simple wrapper
 scripts (from shell or Perl) or via frontends that poll network devices and
 put friendly user interface on it.

 This is version 1.0.x with cgilib-0.4, gd1.3 and libpng-1.0.9 linked into
 librrd.so. The library is much smaller compared to the 1.2.x version with
 separate dynamic linked libraries.
endef

define Package/librrd1
$(call Package/rrdtool1/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+zlib
  TITLE+= management library
endef

define Package/librrd1/description
$(call Package/rrdtool1/description/Default)
 This package contains a shared library, used by other programs.
endef

define Package/rrdcgi1
$(call Package/rrdtool1/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Database
  DEPENDS:=+librrd1
  TITLE+= CGI graphing tool
endef

define Package/rrdcgi1/description
$(call Package/rrdtool1/description/Default)
 This package contains the rrdcgi tool used to create web pages containing
 RRD graphs based on templates.
endef

define Package/rrdtool1
$(call Package/rrdtool1/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Database
  DEPENDS:=+librrd1
  TITLE+= management tools
endef

define Package/rrdtool1/description
$(call Package/rrdtool1/description/Default)
 This package contains command line tools used to manage RRDs.
endef

TARGET_CFLAGS += $(FPIC) --std=c99

CONFIGURE_ARGS += \
	$(DISABLE_NLS) \
	--enable-shared=yes \
	--enable-static=yes \
	--disable-rpath \
	--with-gnu-ld \
	--enable-local-zlib

CONFIGURE_VARS += \
	ac_cv_path_PERL=no \
	rd_cv_ieee_works=yes \
	shrext_cmds=".so"

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		shrext_cmds=".so" \
		all install
endef

define Package/rrdtool1/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/rrd{tool,update} $(1)/usr/bin/
endef

define Package/rrdcgi1/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/rrdcgi $(1)/usr/bin/
endef

define Package/librrd1/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/librrd.so.* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/rrdtool-1.0/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/rrd.h $(1)/usr/lib/rrdtool-1.0/include/
	$(INSTALL_DIR)  $(1)/usr/lib/rrdtool-1.0/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/librrd.{a,so*} $(1)/usr/lib/rrdtool-1.0/lib/
endef

$(eval $(call BuildPackage,librrd1))
$(eval $(call BuildPackage,rrdcgi1))
$(eval $(call BuildPackage,rrdtool1))
