# Copyright (C) 2019 Eneas Ulir de Queiroz <cotequeiroz@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=afalg_engine
PKG_VERSION:=1.2.0-beta.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/cotequeiroz/afalg_engine/archive/v$(PKG_VERSION)
PKG_HASH:=6f0da98a3c12eaf50331ac7cd81f7b8800abf54b96fd73bd3e37cc50fd3d2ba8

PKG_MAINTAINER:=Eneas U de Queiroz <cotequeiroz@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CONFIG_DEPENDS:= \
	CONFIG_AFALG_DIGESTS \
	CONFIG_AFALG_FALLBACK \
	CONFIG_AFALG_UPDATE_CTR_IV \
	CONFIG_AFALG_ZERO_COPY

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

ENGINES_DIR:=engines-1.1

define Package/libopenssl-afalg_sync
    SECTION:=libs
    CATEGORY:=Libraries
    SUBMENU:=SSL
    MDEPENDS:=libopenssl
    TITLE:=AF_ALG engine using sync crypto API
    URL:=https://github.com/cotequeiroz/afalg_engine
    DEPENDS:=libopenssl @OPENSSL_ENGINE @!OPENSSL_ENGINE_BUILTIN_AFALG \
	     +libopenssl-conf +kmod-crypto-user
    CONFLICTS:=libopenssl-afalg $(if $(CONFIG_AFALG_FALLBACK),libopenssl-devcrypto)
    MENU:=1
endef

define Package/libopenssl-afalg_sync/description
    This is an alternate AF_ALG engine for openssl, based on the devcrypto
    engine, but using the AF_ALG interface instead of /dev/crypto

    It is different than the AF_ALG engine that ships with OpenSSL:
     - it is faster
     - it uses sync calls, instead of async
     - it suports more algorithms
endef

define Package/libopenssl-afalg_sync/config
    source "$(SOURCE)/Config.in"
endef

CMAKE_OPTIONS += \
	-DOPENSSL_ENGINES_DIR=/usr/lib/$(ENGINES_DIR) \
	-DDIGESTS=$(if $(CONFIG_AFALG_DIGESTS),ON,OFF) \
	-DFALLBACK=$(if $(CONFIG_AFALG_FALLBACK),ON,OFF) \
	-DUPDATE_CTR_IV=$(if $(CONFIG_AFALG_UPDATE_CTR_IV),ON,OFF) \
	-DUSE_ZERO_COPY=$(if $(CONFIG_AFALG_ZERO_COPY),ON,OFF)

define Package/libopenssl-afalg_sync/install
	$(INSTALL_DIR) $(1)/usr/lib/$(ENGINES_DIR)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/$(ENGINES_DIR)/afalg.so \
			$(1)/usr/lib/$(ENGINES_DIR)/
endef

$(eval $(call BuildPackage,libopenssl-afalg_sync))
