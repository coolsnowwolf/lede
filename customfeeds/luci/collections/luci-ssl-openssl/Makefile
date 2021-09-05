#
# Copyright (C) 2016 The LuCI Team
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TYPE:=col
LUCI_BASENAME:=ssl-openssl

LUCI_TITLE:=LuCI with HTTPS support (OpenSSL as SSL backend)
LUCI_DESCRIPTION:=LuCI with OpenSSL as the SSL backend (libustream-openssl). \
 OpenSSL cmd tools (openssl-util) are used by uhttpd for SSL key generation \
 instead of the default px5g. (If px5g is installed, uhttpd will prefer that.)

LUCI_DEPENDS:=+luci +libustream-openssl +openssl-util

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
