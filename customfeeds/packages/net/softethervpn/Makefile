#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Based on the initial porting done by el1n

include $(TOPDIR)/rules.mk

PKG_NAME:=softethervpn
PKG_VERSION:=4.34-9745
PKG_VERREL:=rtm
PKG_VERDATE:=2020.04.05
PKG_RELEASE:=1

PKG_SOURCE:=softether-src-v$(PKG_VERSION)-$(PKG_VERREL).tar.gz
PKG_SOURCE_URL:=http://www.softether-download.com/files/softether/v$(PKG_VERSION)-$(PKG_VERREL)-$(PKG_VERDATE)-tree/Source_Code/
PKG_HASH:=bf5547e2a190e8620fe02da9756b32d010e3b64cbc6317f172f7820394b4c036

PKG_MAINTAINER:=Federico Di Marco <fededim@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/v$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)/v$(PKG_VERSION)
PKG_BUILD_DEPENDS:=softethervpn/host
HOST_BUILD_DEPENDS:=readline/host libiconv/host

PKG_ASLR_PIE:=0
HAMCORE_SE2:=$(STAGING_DIR_HOST)/share/softethervpn/hamcore.se2

include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

# Execute in host build directory
HOST_MAKE_FLAGS += -C $(HOST_BUILD_DIR)

# Select 32 or 64 bit Makefile for host build depending on host architecture
HOST_MAKE_FLAGS += -f src/makefiles/linux_$(if $(shell uname -m | grep 64),64,32)bit.mak

HOST_LDFLAGS += -Wl,-rpath$(comma)$(STAGING_DIR_HOSTPKG)/lib
# Prevent calling upstream configure
define Host/Configure
endef

define Host/Compile
	# Build hamcorebuilder using host compiler and let it generate
	# the hamcore.se2 archive file
	# CFLAGS, CPPFLAGS & LDFLAGS need to be passed with CC because they are being ingored
	CC="$(HOSTCC) $(HOST_CFLAGS) $(HOST_CPPFLAGS) $(HOST_LDFLAGS)" $(MAKE) $(HOST_MAKE_FLAGS) \
		src/bin/BuiltHamcoreFiles/unix/hamcore.se2
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/share/softethervpn
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/src/bin/BuiltHamcoreFiles/unix/hamcore.se2 $(HAMCORE_SE2)
endef


# Tune CFLAGS for target
TARGET_CFLAGS += \
	$(if $(CONFIG_OPENSSL_WITH_SSL3),,-DSSL_OP_NO_SSLv3) \
	$(if $(filter mips mipsel,$(ARCH)),-minterlink-mips16)

# Select 32 or 64 bit Makefile for target build depending on 64bit config symbol
MAKE_FLAGS += \
	-f src/makefiles/linux_$(if $(CONFIG_ARCH_64BIT),64,32)bit.mak

# Map nonstandard CCFLAGS variable to standard TARGET_CFLAGS
MAKE_FLAGS += \
	CC="$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(TARGET_LDFLAGS)"

define Build/Configure
	# Fetch prebuilt hamcore.se2 from staging dir
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/src/bin/BuiltHamcoreFiles/unix
	$(CP) $(HAMCORE_SE2) $(PKG_BUILD_DIR)/src/bin/BuiltHamcoreFiles/unix/hamcore.se2

	# Portably set hamcore.se2 modtime to one day in the future
	# to prevent rebuilding it
	perl -e 'utime(time() + 86400, time() + 86400, $$$$ARGV[0])' \
		$(PKG_BUILD_DIR)/src/bin/BuiltHamcoreFiles/unix/hamcore.se2
endef


define Package/softethervpn/default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  TITLE:=Free Cross-platform Multi-protocol VPN server and client
  URL:=http://www.softether.org/
endef

define Package/softethervpn/description
SoftEther VPN ("SoftEther" means "Software Ethernet") is one of the world's most powerful and easy-to-use multi-protocol VPN software developed as an academic
project from University of Tsukuba, Japan. SoftEther VPN has strong compatibility to today's most popular VPN products among the world. It has the interoperability
with OpenVPN, L2TP, IPsec, EtherIP, L2TPv3, Cisco VPN Routers and MS-SSTP VPN Clients. SoftEther VPN is the world's only VPN software which supports SSL-VPN,
OpenVPN, L2TP, EtherIP, L2TPv3 and IPsec as a single VPN software. SoftEther VPN is not only an alternative VPN server to existing VPN products (OpenVPN,
IPsec and MS-SSTP), but has also original strong SSL-VPN protocol to penetrate any kinds of firewalls. Ultra-optimized SSL-VPN Protocol of SoftEther VPN
has very fast throughput, low latency and firewall resistance.
endef

define Package/softethervpn-base
  $(Package/softethervpn/default)
  DEPENDS:=+libpthread +librt +libreadline +libopenssl +libncurses +kmod-tun +zlib $(ICONV_DEPENDS)
  TITLE += (Base)
endef

define Package/softethervpn-server
  $(Package/softethervpn/default)
  DEPENDS:=+softethervpn-base
  TITLE += (Server)
endef

define Package/softethervpn-bridge
  $(Package/softethervpn/default)
  DEPENDS:=+softethervpn-base
  TITLE += (Bridge)
endef

define Package/softethervpn-client
  $(Package/softethervpn/default)
  DEPENDS:=+softethervpn-base
  TITLE += (Client)
endef

Package/softethervpn-base/description = $(Package/softethervpn/description)
Package/softethervpn-server/description = $(Package/softethervpn/description)
Package/softethervpn-bridge/description = $(Package/softethervpn/description)
Package/softethervpn-client/description = $(Package/softethervpn/description)

define Package/softethervpn-base/conffiles
/usr/libexec/softethervpn/lang.config
endef

define Package/softethervpn-server/conffiles
/usr/libexec/softethervpn/vpn_server.config
endef

define Package/softethervpn-client/conffiles
/usr/libexec/softethervpn/vpn_client.config
endef

define Package/softethervpn-bridge/conffiles
/usr/libexec/softethervpn/vpn_bridge.config
endef

define Package/softethervpn-base/install
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vpnserver/hamcore.se2 $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vpncmd/vpncmd $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) files/launcher.sh $(1)/usr/libexec/softethervpn

	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/lang.config

	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) ../../usr/libexec/softethervpn/launcher.sh $(1)/usr/bin/vpncmd
endef

define Package/softethervpn-server/install
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vpnserver/vpnserver $(1)/usr/libexec/softethervpn

	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/vpn_server.config

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/vpnserver.init $(1)/etc/init.d/softethervpnserver
endef

define Package/softethervpn-bridge/install
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vpnbridge/vpnbridge $(1)/usr/libexec/softethervpn

	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/vpn_bridge.config

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/vpnbridge.init $(1)/etc/init.d/softethervpnbridge
endef

define Package/softethervpn-client/install
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vpnclient/vpnclient $(1)/usr/libexec/softethervpn

	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/vpn_client.config

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/vpnclient.init $(1)/etc/init.d/softethervpnclient
endef

$(eval $(call BuildPackage,softethervpn-base))
$(eval $(call BuildPackage,softethervpn-server))
$(eval $(call BuildPackage,softethervpn-client))
$(eval $(call BuildPackage,softethervpn-bridge))
$(eval $(call HostBuild))
