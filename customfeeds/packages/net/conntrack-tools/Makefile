#
# Copyright (C) 2009-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=conntrack-tools
PKG_VERSION:=1.4.6
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.netfilter.org/projects/conntrack-tools/files
PKG_HASH:=590859cc848245dbfd9c6487761dd303b3a1771e007f4f42213063ca56205d5f

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:netfilter:conntrack-tools

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=libtirpc

include $(INCLUDE_DIR)/package.mk

define Package/conntrack-tools/default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libnetfilter-conntrack +libnetfilter-cttimeout +libnetfilter-cthelper +libnetfilter-queue
  SUBMENU:=Firewall
  URL:=https://conntrack-tools.netfilter.org/
endef

define Package/conntrack
$(call Package/conntrack-tools/default)
  TITLE:=Connection tracking tool
endef

define Package/conntrack/description
 Conntrack is a userspace command line program targeted at system
 administrators. It enables them to view and manage the in-kernel
 connection tracking state table.
endef

CONFIGURE_ARGS += \
	--with-libtirpc

define Package/conntrack/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/conntrack $(1)/usr/sbin/
endef

define Package/conntrackd
$(call Package/conntrack-tools/default)
  TITLE:=Connection tracking daemon
endef

define Package/conntrackd/conffiles
/etc/conntrackd/
endef

define Package/conntrackd/description
 Conntrackd can replicate the status of the connections that are
 currently being processed by your stateful firewall based on Linux.
 Conntrackd can also run as statistics daemon.
endef

define Package/conntrackd/install
	$(INSTALL_DIR) \
		$(1)/etc/conntrackd \
		$(1)/etc/init.d \
		$(1)/usr/sbin
	$(INSTALL_BIN) 	$(PKG_INSTALL_DIR)/usr/sbin/conntrackd $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/conntrackd.init $(1)/etc/init.d/conntrackd
endef

$(eval $(call BuildPackage,conntrack))
$(eval $(call BuildPackage,conntrackd))
