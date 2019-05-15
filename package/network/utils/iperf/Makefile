#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iperf
PKG_VERSION:=2.0.12
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=367f651fb1264b13f6518e41b8a7e08ce3e41b2a1c80e99ff0347561eed32646
PKG_SOURCE_URL:=@SF/iperf2

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=BSD-3-Clause

PKG_BUILD_PARALLEL:=1

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

TARGET_CFLAGS += -D_GNU_SOURCE
CONFIGURE_ARGS += --disable-multicast

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
