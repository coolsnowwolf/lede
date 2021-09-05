#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=NIXIO POSIX library
LUCI_DEPENDS:=+PACKAGE_luci-lib-nixio_openssl:libopenssl +PACKAGE_luci-lib-nixio_cyassl:libcyassl +liblua

PKG_LICENSE:=Apache-2.0

define Package/luci-lib-nixio/config
	choice
		prompt "TLS Provider"
		default PACKAGE_luci-lib-nixio_openssl

		config PACKAGE_luci-lib-nixio_notls
			bool "Disabled"

		config PACKAGE_luci-lib-nixio_axtls
			bool "Builtin (axTLS)"

		config PACKAGE_luci-lib-nixio_cyassl
			bool "CyaSSL"
			select PACKAGE_libcyassl

		config PACKAGE_luci-lib-nixio_openssl
			bool "OpenSSL"
			select PACKAGE_libopenssl
	endchoice
endef

NIXIO_TLS:=

ifneq ($(CONFIG_PACKAGE_luci-lib-nixio_axtls),)
  NIXIO_TLS:=axtls
endif

ifneq ($(CONFIG_PACKAGE_luci-lib-nixio_openssl),)
  NIXIO_TLS:=openssl
endif

ifneq ($(CONFIG_PACKAGE_luci-lib-nixio_cyassl),)
  NIXIO_TLS:=cyassl
  LUCI_CFLAGS+=-I$(STAGING_DIR)/usr/include/cyassl
endif

MAKE_VARS += NIXIO_TLS="$(NIXIO_TLS)"

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
