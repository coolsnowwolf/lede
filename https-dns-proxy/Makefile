include $(TOPDIR)/rules.mk

PKG_NAME:=https-dns-proxy
PKG_VERSION:=2021-07-29
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/aarond10/https_dns_proxy/
PKG_SOURCE_DATE:=2021-07-29
PKG_SOURCE_VERSION:=750c4dbdfe3d3c65edcd57349df181ffd1f6ceeb
PKG_MIRROR_HASH:=88f487d80f8f20b83d1f9940c30ad1d5b6a4eda6be48fbaea196c69e90c5429e
PKG_MAINTAINER:=Stan Grishin <stangri@melmac.net>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DCLANG_TIDY_EXE=

define Package/https-dns-proxy
	SECTION:=net
	CATEGORY:=Network
	TITLE:=DNS Over HTTPS Proxy
	URL:=https://docs.openwrt.melmac.net/https-dns-proxy/
	DEPENDS:=+libcares +libcurl +libev +ca-bundle
	CONFLICTS:=https_dns_proxy
endef

define Package/https-dns-proxy/description
https-dns-proxy is a light-weight DNS<-->HTTPS, non-caching translation proxy for the RFC 8484 DoH standard.
It receives regular (UDP) DNS requests and issues them via DoH.
Please see https://docs.openwrt.melmac.net/https-dns-proxy/ for more information.
endef

define Package/https-dns-proxy/conffiles
/etc/config/https-dns-proxy
endef

define Package/https-dns-proxy/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d ${1}/etc/config $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/https_dns_proxy $(1)/usr/sbin/https-dns-proxy
	$(INSTALL_BIN) ./files/https-dns-proxy.init $(1)/etc/init.d/https-dns-proxy
	$(SED) "s|^\(PKG_VERSION\).*|\1='$(PKG_VERSION)-$(PKG_RELEASE)'|" $(1)/etc/init.d/https-dns-proxy
	$(INSTALL_CONF) ./files/https-dns-proxy.config $(1)/etc/config/https-dns-proxy
	$(INSTALL_DATA) ./files/https-dns-proxy.hotplug.iface $(1)/etc/hotplug.d/iface/90-https-dns-proxy
endef

$(eval $(call BuildPackage,https-dns-proxy))
