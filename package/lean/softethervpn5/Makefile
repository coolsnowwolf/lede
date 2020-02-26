# Based partially on the versions of el1n and Federico Di Marco

include $(TOPDIR)/rules.mk

PKG_NAME:=softethervpn5
PKG_VERSION:=5.01.9671
PKG_RELEASE:=1

PKG_MAINTAINER:=Andy Walsh <andy.walsh44+github@gmail.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_SOURCE_URL:=https://github.com/SoftEtherVPN/SoftEtherVPN/releases/download/$(PKG_VERSION)/
PKG_SOURCE:=softether-vpn-src-$(PKG_VERSION).tar.gz
PKG_HASH:=56930110ad5e21a3b7c69008bdb3efd368c0ebafc1d0d97b48a76a3563ec8e24

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/SoftEtherVPN-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/SoftEtherVPN-$(PKG_VERSION)

HOST_BUILD_DEPENDS:=ncurses/host readline/host
PKG_BUILD_DEPENDS:=softethervpn5/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/softethervpn5/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  TITLE:=softethervpn5 v$(PKG_VERSION)
  URL:=http://www.softether.org/
endef

define Package/softethervpn5/Default/description
  SoftEther VPN supports SSL-VPN, OpenVPN, L2TP, EtherIP, L2TPv3 and IPsec as a single VPN software.
  SoftEther VPN is not only an alternative VPN server to existing VPN products (OpenVPN, IPsec and MS-SSTP),
  but has also original strong SSL-VPN protocol to penetrate any kinds of firewalls.
  Guide: https://wordpress.tirlins.com/2015/03/setting-up-softether-vpn-on-openwrt/
endef

define Package/softethervpn5-libs
  $(call Package/softethervpn5/Default)
  DEPENDS:=+libpthread +librt +libreadline +libopenssl +libncurses +kmod-tun +zlib $(ICONV_DEPENDS)
  TITLE+= libs
  HIDDEN:=1
endef

define Package/softethervpn5-server
  $(call Package/softethervpn5/Default)
  TITLE+= server
  DEPENDS:= +softethervpn5-libs
endef
define Package/softethervpn5-server/description
  $(call Package/softethervpn5/Default/description)

  Provides the vpnserver (daemon).
endef

define Package/softethervpn5-bridge
  $(call Package/softethervpn5/Default)
  TITLE+= bridge
  DEPENDS:= +softethervpn5-libs  
endef
define Package/softethervpn5-bridge/description
  $(call Package/softethervpn5/Default/description)

  Provides the vpnbridge (daemon).
endef

define Package/softethervpn5-client
  $(call Package/softethervpn5/Default)
  TITLE+= client
  DEPENDS:= +softethervpn5-libs  
endef
define Package/softethervpn5-client/description
  $(call Package/softethervpn5/Default/description)

  Provides the vpnclient.
endef

export USE_MUSL=YES
# BUG: outdated host/include/elf.h
HOST_CFLAGS += $(FPIC) -DAT_HWCAP2=26
TARGET_CFLAGS += $(FPIC)
CMAKE_OPTIONS = -DICONV_LIB_PATH="$(ICONV_PREFIX)/lib"

# static build for host (hamcorebuilder), avoid -fpic on ncurses/host and shared libs can't be found on host
define Host/Prepare
	$(Host/Prepare/Default)
	$(SED) 's,SHARED,STATIC,g' $(HOST_BUILD_DIR)/src/Mayaqua/CMakeLists.txt
	$(SED) 's,SHARED,STATIC,g' $(HOST_BUILD_DIR)/src/Cedar/CMakeLists.txt
	$(SED) 's,readline,libreadline.a,g' $(HOST_BUILD_DIR)/src/Cedar/CMakeLists.txt
endef

define Host/Compile
	$(call Host/Compile/Default,hamcorebuilder)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/tmp/hamcorebuilder $(STAGING_DIR_HOSTPKG)/bin/
endef

define Build/Compile
	$(call Build/Compile/Default,vpnserver vpnbridge vpnclient vpncmd hamcore-archive-build)
endef

define Build/Install
endef

define Package/softethervpn5-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/build/libcedar.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/build/libmayaqua.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(CP) $(PKG_BUILD_DIR)/build/hamcore.se2 $(1)/usr/libexec/softethervpn/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/vpncmd $(1)/usr/libexec/softethervpn/
	$(INSTALL_BIN) files/launcher.sh $(1)/usr/libexec/softethervpn/
	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/lang.config
	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) ../../usr/libexec/softethervpn/launcher.sh $(1)/usr/bin/vpncmd
endef

define Package/softethervpn5-server/install
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/vpnserver $(1)/usr/libexec/softethervpn/
	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/vpn_server.config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/vpnserver.init $(1)/etc/init.d/softethervpnserver
endef

define Package/softethervpn5-bridge/install
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/vpnbridge $(1)/usr/libexec/softethervpn/
	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/vpn_bridge.config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/vpnbridge.init $(1)/etc/init.d/softethervpnbridge
endef

define Package/softethervpn5-client/install
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/vpnclient $(1)/usr/libexec/softethervpn/
	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/vpn_client.config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/vpnclient.init $(1)/etc/init.d/softethervpnclient
endef

define Package/softethervpn5-server/conffiles
/usr/libexec/softethervpn/vpn_server.config
/usr/libexec/softethervpn/lang.config
endef

define Package/softethervpn5-bridge/conffiles
/usr/libexec/softethervpn/vpn_bridge.config
/usr/libexec/softethervpn/lang.config
endef

define Package/softethervpn5-client/conffiles
/usr/libexec/softethervpn/vpn_client.config
/usr/libexec/softethervpn/lang.config
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,softethervpn5-libs))
$(eval $(call BuildPackage,softethervpn5-server))
$(eval $(call BuildPackage,softethervpn5-bridge))
$(eval $(call BuildPackage,softethervpn5-client))
