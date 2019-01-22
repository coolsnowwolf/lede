include $(TOPDIR)/rules.mk

PKG_NAME:=shadowsocksR-libev
PKG_VERSION:=20180307
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.gz
PKG_SOURCE_URL:=https://github.com/shadowsocksrr/shadowsocksr-libev.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=138fd46f8057142b4aa4d86b7464f17ff09b7a67
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_MAINTAINER:=Akkariiin

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/shadowsocksr-libev/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Lightweight Secured Socks5 Proxy
  URL:=https://github.com/shadowsocksr/shadowsocksr-libev
endef

define Package/shadowsocksr-libev
  $(call Package/shadowsocksr-libev/Default)
  TITLE+= (OpenSSL)
  VARIANT:=openssl
  DEPENDS:=+libopenssl +libpcre +libpthread +zlib
endef


define Package/shadowsocksr-libev-mbedtls
  $(call Package/shadowsocksr-libev/Default)
  TITLE+= (mbedTLS)
  VARIANT:=mbedtls
  DEPENDS:=+libpthread +libpcre +libmbedtls
endef


define Package/shadowsocksr-libev/description
ShadowsocksR-libev is a lightweight secured socks5 proxy for embedded devices and low end boxes.
endef


Package/shadowsocksr-libev-mbedtls/description=$(Package/shadowsocksr-libev/description)

define Package/shadowsocksr-libev/conffiles
/etc/shadowsocksr.json
endef

Package/shadowsocksr-libev-polarssl/conffiles = $(Package/shadowsocksr-libev/conffiles)
Package/shadowsocksr-libev-mbedtls/conffiles = $(Package/shadowsocksr-libev/conffiles)

define Package/shadowsocksr-libev-server/conffiles
/etc/shadowsocksr-server.json
endef

Package/shadowsocksr-libev-server-mbedtls/conffiles = $(Package/shadowsocksr-libev-server/conffiles)


CONFIGURE_ARGS += --disable-ssp --disable-documentation --disable-assert 

ifeq ($(BUILD_VARIANT),openssl)
	CONFIGURE_ARGS += --with-crypto-library=openssl
endif

ifeq ($(BUILD_VARIANT),mbedtls)
	CONFIGURE_ARGS += --with-crypto-library=mbedtls
endif

define Package/shadowsocksr-libev/install
	$(INSTALL_DIR) $(1)/etc/init.d
	#$(INSTALL_BIN) ./files/shadowsocksr $(1)/etc/init.d/shadowsocksr
	$(INSTALL_CONF) ./files/shadowsocksr.json $(1)/etc/shadowsocksr.json
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-local $(1)/usr/bin/ssr-local
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ss-redir $(1)/usr/bin/ssr-redir
endef

Package/shadowsocksr-libev-polarssl/install=$(Package/shadowsocksr-libev/install)
Package/shadowsocksr-libev-mbedtls/install=$(Package/shadowsocksr-libev/install)

Package/shadowsocksr-libev-server-mbedtls/install=$(Package/shadowsocksr-libev-server/install)

$(eval $(call BuildPackage,shadowsocksr-libev))
$(eval $(call BuildPackage,shadowsocksr-libev-mbedtls))
