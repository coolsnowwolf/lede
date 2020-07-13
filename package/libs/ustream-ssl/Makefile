include $(TOPDIR)/rules.mk

PKG_NAME:=ustream-ssl
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/ustream-ssl.git
PKG_SOURCE_DATE:=2020-03-13
PKG_SOURCE_VERSION:=5e1bc3429cbf9c3be4db65ef5dbf21ea99cf5b95
PKG_MIRROR_HASH:=c59dea9b98d3ce88d886f7c7b3b252c55312ad281b731ab9172ae78570f1b643
CMAKE_INSTALL:=1

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libustream/default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=ustream SSL Library
  DEPENDS:=+libubox
  ABI_VERSION:=20200215
endef

define Package/libustream-openssl
  $(Package/libustream/default)
  TITLE += (openssl)
  DEPENDS += +PACKAGE_libustream-openssl:libopenssl
  VARIANT:=openssl
endef

define Package/libustream-wolfssl
  $(Package/libustream/default)
  TITLE += (wolfssl)
  DEPENDS += +PACKAGE_libustream-wolfssl:libwolfssl
  VARIANT:=wolfssl
endef

define Package/libustream-mbedtls
  $(Package/libustream/default)
  TITLE += (mbedtls)
  DEPENDS += +libmbedtls
  VARIANT:=mbedtls
  DEFAULT_VARIANT:=1
endef

ifeq ($(BUILD_VARIANT),wolfssl)
  TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/wolfssl
  CMAKE_OPTIONS += -DWOLFSSL=on
endif
ifeq ($(BUILD_VARIANT),mbedtls)
  CMAKE_OPTIONS += -DMBEDTLS=on
endif

define Package/libustream/default/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libustream-ssl.so $(1)/lib/
endef

Package/libustream-openssl/install = $(Package/libustream/default/install)
Package/libustream-wolfssl/install = $(Package/libustream/default/install)
Package/libustream-mbedtls/install = $(Package/libustream/default/install)

$(eval $(call BuildPackage,libustream-mbedtls))
$(eval $(call BuildPackage,libustream-wolfssl))
$(eval $(call BuildPackage,libustream-openssl))
