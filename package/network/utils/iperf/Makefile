#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iperf
PKG_VERSION:=2.0.13
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=c88adec966096a81136dda91b4bd19c27aae06df4d45a7f547a8e50d723778ad
PKG_SOURCE_URL:=@SF/iperf2

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=BSD-3-Clause

PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS:=CONFIG_IPERF_ENABLE_MULTICAST

include $(INCLUDE_DIR)/uclibc++.mk
include $(INCLUDE_DIR)/package.mk

define Package/iperf
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:= $(CXX_DEPENDS) +libpthread
  TITLE:=Internet Protocol bandwidth measuring tool
  URL:=http://sourceforge.net/projects/iperf2/
endef

define Package/iperf/description
 Iperf is a modern alternative for measuring TCP and UDP bandwidth
 performance, allowing the tuning of various parameters and
 characteristics.
endef

define Package/iperf/config
	config IPERF_ENABLE_MULTICAST
		depends on PACKAGE_iperf
		bool "Enable multicast support"
endef


TARGET_CFLAGS += -D_GNU_SOURCE
ifeq ($(CONFIG_IPERF_ENABLE_MULTICAST),y)
CONFIGURE_ARGS += --enable-multicast
else
CONFIGURE_ARGS += --disable-multicast
endif

ifeq ($(CONFIG_IPV6),)
	CONFIGURE_ARGS += --disable-ipv6
endif

CONFIGURE_VARS += CXXFLAGS="$$$$CXXFLAGS -fno-rtti"
CONFIGURE_VARS += LIBS="-lpthread -lm"

define Package/iperf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/iperf $(1)/usr/bin/iperf
endef

$(eval $(call BuildPackage,iperf))
