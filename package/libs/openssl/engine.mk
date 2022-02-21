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
# $$$$1 == non-empty: suggest reinstall
error_out() {
    [ "$1" ] && cat <<- EOF
	Reinstalling the libopenssl-conf package may fix this:

	    opkg install --force-reinstall libopenssl-conf
	EOF
    cat <<- EOF

	Then, you will have to reinstall this package, and any other engine package you have
	you have previously installed to ensure they are enabled:

	    opkg install --force-reinstall $$(OSSL_ENG_PKG) [OTHER_ENGINE_PKG]...

	EOF
    exit 1
}
ENGINES_CNF="$$$${IPKG_INSTROOT}/etc/ssl/engines.cnf.d/engines.cnf"
OPENSSL_CNF="$$$${IPKG_INSTROOT}/etc/ssl/openssl.cnf"
if [ ! -f "$$$${OPENSSL_CNF}" ]; then
    echo -e "ERROR: File $$$${OPENSSL_CNF} not found."
    error_out reinstall
fi
if ! grep -q "^.include /etc/ssl/engines.cnf.d" "$$$${OPENSSL_CNF}"; then
    cat <<- EOF
	Your /etc/ssl/openssl.cnf file is not loading engine configuration files from
	/etc/ssl/engines.cnf.d.  You should consider start with a fresh, updated OpenSSL config by
	running:

	    opkg install --force-reinstall --force-maintainer libopenssl-conf

	The above command will overwrite any changes you may have made to both /etc/ssl/openssl.cnf
	and /etc/ssl/engines.cnf.d/engines.cnf files, so back them up first!
	EOF
    error_out
fi
if [ ! -f "$$$${ENGINES_CNF}" ]; then
    echo "Can't configure $$(OSSL_ENG_PKG): File $$$${ENGINES_CNF} not found."
    error_out reinstall
fi
if grep -q "$(1)=$(1)" "$$$${ENGINES_CNF}"; then
    echo "$$(OSSL_ENG_PKG): $(1) engine was already configured.  Nothing to be done."
else
    echo "$(1)=$(1)" >> "$$$${ENGINES_CNF}"
    echo "$$(OSSL_ENG_PKG): $(1) engine enabled.  All done!"
fi
  endef

  define Package/$$(OSSL_ENG_PKG)/prerm :=
#!/bin/sh
ENGINES_CNF="$$$${IPKG_INSTROOT}/etc/ssl/engines.cnf.d/engines.cnf"
[ -f "$$$${ENGINES_CNF}" ] || exit 0
sed -e '/$(1)=$(1)/d' -i "$$$${ENGINES_CNF}"
  endef
endef


