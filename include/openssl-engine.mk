# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 Enéas Ulir de Queiroz

ENGINES_DIR=engines-1.1

define Package/openssl/engine/Default
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=SSL
  DEPENDS:=libopenssl @OPENSSL_ENGINE +libopenssl-conf
endef

# 1 = engine name
# 2 - package name, defaults to libopenssl-$(1)
define Package/openssl/add-engine
  OSSL_ENG_PKG:=$(if $(2),$(2),libopenssl-$(1))
  Package/$$(OSSL_ENG_PKG)/conffiles:=/etc/ssl/engines.cnf.d/$(1).cnf

  define Package/$$(OSSL_ENG_PKG)/install
	$$(INSTALL_DIR)  $$(1)/usr/lib/$(ENGINES_DIR)
	$$(INSTALL_BIN)  $$(PKG_INSTALL_DIR)/usr/lib/$(ENGINES_DIR)/$(1).so \
			 $$(1)/usr/lib/$(ENGINES_DIR)
	$$(INSTALL_DIR)  $$(1)/etc/ssl/engines.cnf.d
	$$(INSTALL_DATA) ./files/$(1).cnf $$(1)/etc/ssl/engines.cnf.d/
  endef

  define Package/$$(OSSL_ENG_PKG)/postinst :=
#!/bin/sh
OPENSSL_UCI="$$$${IPKG_INSTROOT}/etc/config/openssl"

[ -z "$$$${IPKG_INSTROOT}" ] && uci -q get openssl.$(1) >/dev/null && exit 0

cat << EOF >> "$$$${OPENSSL_UCI}"

config engine '$(1)'
	option enabled '1'
EOF

[ -n "$$$${IPKG_INSTROOT}" ] || /etc/init.d/openssl reload
  endef

  define Package/$$(OSSL_ENG_PKG)/postrm :=
#!/bin/sh
[ -n "$$$${IPKG_INSTROOT}" ] && exit 0
uci delete openssl.$(1)
uci commit openssl
/etc/init.d/openssl reload
  endef
endef
