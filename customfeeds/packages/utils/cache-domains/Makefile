include $(TOPDIR)/rules.mk

PKG_NAME:=cache-domains
PKG_VERSION:=2.2.1
PKG_RELEASE:=1

PKG_MAINTAINER:=Gerard Ryan <G.M0N3Y.2503@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/cache-domains/default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Game content cache DNS
  URL:=https://github.com/uklans/cache-domains
  DEPENDS:=+jq +bash +dnsmasq +ca-bundle
endef

define Package/cache-domains/description/default
hotplug script to dynamically configure the local DNS (dnsmasq) to redirect game content servers to a LAN cache.
Definitive list dynamically obtained from https://github.com/uklans/cache-domains.
endef

define Package/cache-domains/install/default
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/cache-domains $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface/
	$(INSTALL_BIN) ./files/30-cache-domains $(1)/etc/hotplug.d/iface/
endef

Build/Compile=# Nothing to compile, just install the scripts


define Package/cache-domains-openssl
  $(Package/cache-domains/default)
  TITLE += (openssl)
  DEPENDS += +libustream-openssl
  VARIANT:=openssl
endef
Package/cache-domains-openssl/description = $(Package/cache-domains/description/default)
Package/cache-domains-openssl/install = $(Package/cache-domains/install/default)

define Package/cache-domains-mbedtls
  $(Package/cache-domains/default)
  TITLE += (mbedtls)
  DEPENDS += +libustream-mbedtls
  VARIANT:=mbedtls
endef
Package/cache-domains-mbedtls/description = $(Package/cache-domains/description/default)
Package/cache-domains-mbedtls/install = $(Package/cache-domains/install/default)

define Package/cache-domains-wolfssl
  $(Package/cache-domains/default)
  TITLE += (wolfssl)
  DEPENDS += +libustream-wolfssl
  VARIANT:=wolfssl
  DEFAULT_VARIANT:=1
endef
Package/cache-domains-wolfssl/description = $(Package/cache-domains/description/default)
Package/cache-domains-wolfssl/install = $(Package/cache-domains/install/default)

$(eval $(call BuildPackage,cache-domains-openssl))
$(eval $(call BuildPackage,cache-domains-mbedtls))
$(eval $(call BuildPackage,cache-domains-wolfssl))
