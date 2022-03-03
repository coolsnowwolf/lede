# Copyright (C) 2020 Paul Spooren <mail@aparcar.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=px5g-wolfssl
PKG_RELEASE:=$(COMMITCOUNT)
PKG_LICENSE:=GPL-2.0-or-later

PKG_USE_MIPS16:=0

PKG_MAINTAINER:=Paul Spooren <mail@aparcar.org>

PKG_CONFIG_DEPENDS:=CONFIG_WOLFSSL_ALT_NAMES

include $(INCLUDE_DIR)/package.mk

define Package/px5g-wolfssl
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE:=X.509 certificate generator (using WolfSSL)
  DEPENDS:=+libwolfssl
  PROVIDES:=px5g
  VARIANT:=wolfssl
endef

define Package/px5g-wolfssl/description
 Px5g is a tiny X.509 certificate generator.
 It suitable to create key files and certificates in DER
 and PEM format for use with stunnel, uhttpd and others.
endef

TARGET_LDFLAGS += -lwolfssl


TARGET_CFLAGS += -Wl,--gc-sections

define Build/Compile
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/px5g px5g-wolfssl.c $(TARGET_LDFLAGS)
endef

define Package/px5g-wolfssl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/px5g $(1)/usr/sbin/px5g
endef

$(eval $(call BuildPackage,px5g-wolfssl))
