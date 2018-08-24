#
# Copyright (C) 2010-2015 Jo-Philipp Wich <jo@mein.io>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=px5g
PKG_RELEASE:=6
PKG_LICENSE:=LGPL-2.1
PKG_BUILD_DIR:=$(BUILD_DIR)/px5g-$(BUILD_VARIANT)

PKG_USE_MIPS16:=0

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

include $(INCLUDE_DIR)/package.mk

define Package/px5g-mbedtls
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE:=X.509 certificate generator (using mbedtls)
  DEPENDS:=+libmbedtls
  PROVIDES:=px5g
  VARIANT:=mbedtls
endef

define Package/px5g-mbedtls/description
 Px5g is a tiny standalone X.509 certificate generator.
 It suitable to create key files and certificates in DER
 and PEM format for use with stunnel, uhttpd and others.
endef

define Package/px5g-standalone
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE:=X.509 certificate generator (standalone)
  VARIANT:=standalone
endef
Package/px5g-standalone/description = $(Package/px5g-mbedtls/description)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

TARGET_LDFLAGS += -lmbedtls -lmbedx509 -lmbedcrypto

ifeq ($(BUILD_VARIANT),standalone)
  TARGET_LDFLAGS := -Wl,-Bstatic $(TARGET_LDFLAGS) -Wl,-Bdynamic
endif

TARGET_CFLAGS += -Wl,--gc-sections

define Build/Compile
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/px5g px5g.c $(TARGET_LDFLAGS)
endef

define Package/px5g-mbedtls/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/px5g $(1)/usr/sbin/px5g
endef

Package/px5g-standalone/install = $(Package/px5g-mbedtls/install)

$(eval $(call BuildPackage,px5g-mbedtls))
$(eval $(call BuildPackage,px5g-standalone))
