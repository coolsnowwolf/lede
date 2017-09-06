#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iperf
PKG_VERSION:=2.0.9
PKG_VERSION_SUFFIX:=a
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)$(PKG_VERSION_SUFFIX).tar.gz
# Temporary URL change - iperf upstream altered the already released target
# without bumping the version number. The new tarball is renamed to avoid
# conflicts with existing mirrored files.
# PKG_SOURCE_URL:=@SF/iperf2
PKG_SOURCE_URL:=http://mirror2.openwrt.org/sources
PKG_HASH:=db02911f35686e808ed247160dfa766e08ae3f59d1e7dcedef0ffb2a6643f0bf

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

CONFIGURE_VARS += CXXFLAGS="$$$$CXXFLAGS -fno-rtti"
CONFIGURE_VARS += LIBS="-lpthread -lm"

define Package/iperf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/iperf $(1)/usr/bin/iperf
endef

$(eval $(call BuildPackage,iperf))
