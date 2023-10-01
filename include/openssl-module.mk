# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022-2023 Enéas Ulir de Queiroz

ENGINES_DIR=engines-3

define Package/openssl/module/Default
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=SSL
  DEPENDS:=libopenssl +libopenssl-conf
endef

define Package/openssl/engine/Default
  $(Package/openssl/module/Default)
  DEPENDS+=@OPENSSL_ENGINE
endef


# 1 = moudule type (engine|provider)
# 2 = module name
# 3 = directory to save .so file
# 4 = [ package name, defaults to libopenssl-$(2) ]
define Package/openssl/add-module
  $(eval MOD_TYPE:=$(1))
  $(eval MOD_NAME:=$(2))
  $(eval MOD_DIR:=$(3))
  $(eval OSSL_PKG:=$(if $(4),$(4),libopenssl-$(MOD_NAME)))
  $(info Package/openssl/add-module 1='$(1)'; 2='$(2)'; 3='$(3)' 4='$(4)')
  $(info MOD_TYPE='$(MOD_TYPE)'; MOD_NAME='$(MOD_NAME)'; MOD_DIR='$(MOD_DIR)' OSSL_PKG='$(OSSL_PKG)')
  Package/$(OSSL_PKG)/conffiles:=/etc/ssl/modules.cnf.d/$(MOD_NAME).cnf

  define Package/$(OSSL_PKG)/install
	$$(INSTALL_DIR)  $$(1)/$(MOD_DIR)
	$$(INSTALL_BIN)  $$(PKG_INSTALL_DIR)/$(MOD_DIR)/$(MOD_NAME).so \
			 $$(1)/$(MOD_DIR)
	$$(INSTALL_DIR)  $$(1)/etc/ssl/modules.cnf.d
	$$(INSTALL_DATA) ./files/$(MOD_NAME).cnf $$(1)/etc/ssl/modules.cnf.d/
  endef

  define Package/$(OSSL_PKG)/postinst
#!/bin/sh
OPENSSL_UCI="$$$${IPKG_INSTROOT}/etc/config/openssl"

[ -z "$$$${IPKG_INSTROOT}" ] \
	&& uci -q get openssl.$(MOD_NAME) >/dev/null \
	&& exit 0

cat << EOF >> "$$$${OPENSSL_UCI}"

config $(MOD_TYPE) '$(MOD_NAME)'
	option enabled '1'
EOF

[ -n "$$$${IPKG_INSTROOT}" ] || /etc/init.d/openssl reload
exit 0
  endef

  define Package/$(OSSL_PKG)/postrm
#!/bin/sh
[ -n "$$$${IPKG_INSTROOT}" ] && exit 0
uci -q delete openssl.$(MOD_NAME) && uci commit openssl
/etc/init.d/openssl reload
exit 0
  endef
endef

# 1 = engine name
# 2 - package name, defaults to libopenssl-$(1)
define Package/openssl/add-engine
  $(call Package/openssl/add-module,engine,$(1),/usr/lib/$(ENGINES_DIR),$(2))
endef

# 1 = provider name
# 2 = [ package name, defaults to libopenssl-$(1) ]
define Package/openssl/add-provider
  $(call Package/openssl/add-module,provider,$(1),/usr/lib/ossl-modules,$(2))
endef

