#
# Copyright (C) 2019 - Lucian Cristian <lucian.cristian@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openfortivpn
PKG_VERSION:=1.15.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/adrienverge/openfortivpn/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=5279dbd3da779b818d201bbd9243cff9421a7e790fd2190438610a03c88641f8

PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later OpenSSL
PKG_LICENSE_FILES:=LICENSE LICENSE.OpenSSL

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/openfortivpn
  SUBMENU:=VPN
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Fortinet SSL VPN client
  URL:=https://github.com/adrienverge/openfortivpn
  DEPENDS:=+ppp +libopenssl
endef

define Package/openfortivpn/description
  An open implementation of Fortinet's proprietary PPP+SSL VPN solution
  It spawns a pppd process and operates the communication between the gateway and this process.
  It is compatible with Fortinet VPNs.
endef

CONFIGURE_ARGS += \
	--enable-proc \
	--with-rt_dst="yes" \
	--disable-resolvconf \
	--with-resolvconf=DISABLED \
	--with-pppd="/usr/sbin/pppd"

TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Package/openfortivpn/install
	$(INSTALL_DIR) \
	  $(1)/usr/sbin \
          $(1)/lib/netifd/proto \
	  $(1)/etc/hotplug.d/iface

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/openfortivpn $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/openfortivpn-wrapper $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/openfortivpn.sh $(1)/lib/netifd/proto/
	$(INSTALL_BIN) ./files/openfortivpn-ppp-up $(1)/lib/netifd/openfortivpn-ppp-up
endef

$(eval $(call BuildPackage,openfortivpn))
