#
# Copyright (C) 2012-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=strongswan
PKG_VERSION:=5.9.3
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://download.strongswan.org/ https://download2.strongswan.org/
PKG_HASH:=9325ab56a0a4e97e379401e1d942ce3e0d8b6372291350ab2caae0755862c6f7
PKG_LICENSE:=GPL-2.0-or-later
PKG_MAINTAINER:=Philip Prindeville <philipp@redfish-solutions.com>, Noel Kuntze <noel.kuntze@thermi.consulting>
PKG_CPE_ID:=cpe:/a:strongswan:strongswan

PKG_MOD_AVAILABLE:= \
	addrblock \
	aes \
	af-alg \
	agent \
	attr \
	attr-sql \
	bliss \
	blowfish \
	ccm \
	chapoly \
	cmac \
	constraints \
	connmark \
	coupling \
	ctr \
	curl \
	curve25519 \
	des \
	dhcp \
	dnskey \
	duplicheck \
	eap-identity \
	eap-md5 \
	eap-mschapv2 \
	eap-radius \
	eap-tls \
	farp \
	fips-prf \
	forecast \
	gcm \
	gcrypt \
	gmp \
	gmpdh \
	ha \
	hmac \
	kernel-libipsec \
	kernel-netlink \
	ldap \
	led \
	load-tester \
	nonce \
	md4 \
	md5 \
	mgf1 \
	mysql \
	newhope \
	ntru \
	openssl \
	pem \
	pgp \
	pkcs1 \
	pkcs7 \
	pkcs8 \
	pkcs11 \
	pkcs12 \
	pubkey \
	random \
	rc2 \
	resolve \
	revocation \
	sha1 \
	sha2 \
	sha3 \
	smp \
	socket-default \
	socket-dynamic \
	sql \
	sqlite \
	sshkey \
	stroke \
	test-vectors \
	unity \
	uci \
	updown \
	vici \
	whitelist \
	x509 \
	xauth-eap \
	xauth-generic \
	xcbc

PKG_CONFIG_DEPENDS:= \
	CONFIG_STRONGSWAN_ROUTING_TABLE \
	CONFIG_STRONGSWAN_ROUTING_TABLE_PRIO \
	$(patsubst %,CONFIG_PACKAGE_strongswan-mod-%,$(PKG_MOD_AVAILABLE)) \

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
# strongswan-mod-mysql needs iconv
include $(INCLUDE_DIR)/nls.mk

define Package/strongswan/Default
  SUBMENU:=VPN
  SECTION:=net
  CATEGORY:=Network
  TITLE:=StrongSwan
  URL:=http://www.strongswan.org/
endef

define Package/strongswan/description/Default
 StrongSwan is an OpenSource IPsec implementation for the Linux operating system.
endef

define Package/strongswan
$(call Package/strongswan/Default)
  MENU:=1
  DEPENDS:= +libpthread +ip \
	+kmod-crypto-authenc \
	+kmod-ipsec +kmod-ipsec4 +IPV6:kmod-ipsec6 \
	+kmod-ipt-ipsec +iptables-mod-ipsec
endef

define Package/strongswan/config
  source "$(SOURCE)/Config.in"
endef

define Package/strongswan/description
$(call Package/strongswan/description/Default)
 This package contains shared libraries and scripts.
endef

define Package/strongswan-full
$(call Package/strongswan/Default)
  TITLE+= (full)
  DEPENDS:= strongswan \
	+strongswan-charon \
	+strongswan-charon-cmd \
	+strongswan-ipsec \
	+strongswan-libnttfft \
	+strongswan-mod-addrblock \
	+strongswan-mod-aes \
	+strongswan-mod-af-alg \
	+strongswan-mod-agent \
	+strongswan-mod-attr \
	+strongswan-mod-attr-sql \
	+strongswan-mod-bliss \
	+strongswan-mod-blowfish \
	+strongswan-mod-ccm \
	+strongswan-mod-chapoly \
	+strongswan-mod-cmac \
	+strongswan-mod-constraints \
	+strongswan-mod-connmark \
	+strongswan-mod-coupling \
	+strongswan-mod-ctr \
	+strongswan-mod-curl \
	+strongswan-mod-curve25519 \
	+strongswan-mod-des \
	+strongswan-mod-dhcp \
	+strongswan-mod-dnskey \
	+strongswan-mod-duplicheck \
	+strongswan-mod-eap-identity \
	+strongswan-mod-eap-md5 \
	+strongswan-mod-eap-mschapv2 \
	+strongswan-mod-eap-radius \
	+strongswan-mod-eap-tls \
	+strongswan-mod-farp \
	+strongswan-mod-fips-prf \
	+strongswan-mod-forecast \
	+strongswan-mod-gcm \
	+strongswan-mod-gcrypt \
	+strongswan-mod-gmp \
	+strongswan-mod-ha \
	+strongswan-mod-hmac \
	+strongswan-mod-kernel-netlink \
	+strongswan-mod-ldap \
	+strongswan-mod-led \
	+strongswan-mod-load-tester \
	+strongswan-mod-nonce \
	+strongswan-mod-md4 \
	+strongswan-mod-md5 \
	+strongswan-mod-mgf1 \
	+strongswan-mod-mysql \
	+strongswan-mod-newhope \
	+strongswan-mod-ntru \
	+strongswan-mod-openssl \
	+strongswan-mod-pem \
	+strongswan-mod-pgp \
	+strongswan-mod-pkcs1 \
	+strongswan-mod-pkcs7 \
	+strongswan-mod-pkcs8 \
	+strongswan-mod-pkcs11 \
	+strongswan-mod-pkcs12 \
	+strongswan-mod-pubkey \
	+strongswan-mod-random \
	+strongswan-mod-rc2 \
	+strongswan-mod-resolve \
	+strongswan-mod-revocation \
	+strongswan-mod-sha1 \
	+strongswan-mod-sha2 \
	+strongswan-mod-sha3 \
	+strongswan-mod-smp \
	+strongswan-mod-socket-default \
	+strongswan-mod-sql \
	+strongswan-mod-sqlite \
	+strongswan-mod-sshkey \
	+strongswan-mod-stroke \
	+strongswan-mod-test-vectors \
	+strongswan-mod-uci \
	+strongswan-mod-unity \
	+strongswan-mod-updown \
	+strongswan-mod-vici \
	+strongswan-mod-whitelist \
	+strongswan-mod-x509 \
	+strongswan-mod-xauth-eap \
	+strongswan-mod-xauth-generic \
	+strongswan-mod-xcbc \
	+strongswan-pki \
	+strongswan-scepclient \
	+strongswan-swanctl \
	@DEVEL
endef

define Package/strongswan-full/description
$(call Package/strongswan/description/Default)
 This meta-package contains dependencies for all of the strongswan plugins
 except kernel-libipsec,
 socket-dynamic and which are omitted in favor of the kernel-netlink and
 socket-default plugins.
endef


define Package/strongswan-default
$(call Package/strongswan/Default)
  TITLE+= (default)
  DEPENDS:= strongswan \
	+strongswan-charon \
	+strongswan-mod-aes \
	+strongswan-mod-attr \
	+strongswan-mod-connmark \
	+strongswan-mod-constraints \
	+strongswan-mod-des \
	+strongswan-mod-dnskey \
	+strongswan-mod-fips-prf \
	+strongswan-mod-gmp \
	+strongswan-mod-hmac \
	+strongswan-mod-kernel-netlink \
	+strongswan-mod-md5 \
	+strongswan-mod-nonce \
	+strongswan-mod-pem \
	+strongswan-mod-pgp \
	+strongswan-mod-pkcs1 \
	+strongswan-mod-pubkey \
	+strongswan-mod-random \
	+strongswan-mod-rc2 \
	+strongswan-mod-resolve \
	+strongswan-mod-revocation \
	+strongswan-mod-sha1 \
	+strongswan-mod-sha2 \
	+strongswan-mod-socket-default \
	+strongswan-mod-sshkey \
	+strongswan-mod-updown \
	+strongswan-mod-x509 \
	+strongswan-mod-xauth-generic \
	+strongswan-mod-xcbc \
	+strongswan-swanctl
endef

define Package/strongswan-default/description
$(call Package/strongswan/description/Default)
 This meta-package contains only dependencies to match upstream defaults.
endef


define Package/strongswan-isakmp
$(call Package/strongswan/Default)
  TITLE+= (isakmp)
  DEPENDS:= strongswan \
	+strongswan-charon \
	+strongswan-ipsec \
	+strongswan-mod-aes \
	+strongswan-mod-des \
	+strongswan-mod-gmpdh \
	+strongswan-mod-hmac \
	+strongswan-mod-kernel-netlink \
	+strongswan-mod-md5 \
	+strongswan-mod-nonce \
	+strongswan-mod-pubkey \
	+strongswan-mod-random \
	+strongswan-mod-sha1 \
	+strongswan-mod-socket-default \
	+strongswan-mod-stroke \
	+strongswan-mod-uci \
	+strongswan-mod-updown
endef

define Package/strongswan-isakmp/description
$(call Package/strongswan/description/Default)
 This meta-package contains only dependencies to establish  ISAKMP /
 IKE PSK connections, dropping other capabilities in favor of small size 
 Can fit most routers even with 4Mb flash (after removing IPv6 support).
endef


define Package/strongswan-minimal
$(call Package/strongswan/Default)
  TITLE+= (minimal)
  DEPENDS:= strongswan \
	+strongswan-charon \
	+strongswan-mod-aes \
	+strongswan-mod-gmp \
	+strongswan-mod-hmac \
	+strongswan-mod-kernel-netlink \
	+strongswan-mod-nonce \
	+strongswan-mod-pubkey \
	+strongswan-mod-random \
	+strongswan-mod-sha1 \
	+strongswan-mod-socket-default \
	+strongswan-mod-stroke \
	+strongswan-mod-updown \
	+strongswan-mod-x509 \
	+strongswan-mod-xcbc
endef

define Package/strongswan-minimal/description
$(call Package/strongswan/description/Default)
 This meta-package contains only dependencies for a minimal IKEv2 setup.
endef

define Package/strongswan-charon
$(call Package/strongswan/Default)
  TITLE+= IKEv1/IKEv2 keying daemon
  DEPENDS:= strongswan
endef

define Package/strongswan-charon/description
$(call Package/strongswan/description/Default)
 This package contains charon, an IKEv2 keying daemon.
endef

define Package/strongswan-charon-cmd
$(call Package/strongswan/Default)
  TITLE+= charon-cmd utility
  DEPENDS:= strongswan +strongswan-charon
endef

define Package/strongswan-charon-cmd/description
$(call Package/strongswan/description/Default)
 This package contains the charon-cmd utility.
endef

define Package/strongswan-ipsec
$(call Package/strongswan/Default)
  TITLE+= utilities
  DEPENDS:= strongswan
endef

define Package/strongswan-ipsec/description
$(call Package/strongswan/description/Default)
 This package contains the ipsec utility.
endef

define Package/strongswan-libnttfft
$(call Package/strongswan/Default)
  TITLE+= nttfft library
  DEPENDS:= strongswan
endef

define Package/strongswan-libnttfft/description
$(call Package/strongswan/description/Default)
 This package contains the Number Theoretic Transforms library.
endef

define Package/strongswan-pki
$(call Package/strongswan/Default)
  TITLE+= PKI tool
  DEPENDS:= strongswan
endef

define Package/strongswan-pki/description
$(call Package/strongswan/description/Default)
 This package contains the pki tool.
endef

define Package/strongswan-scepclient
$(call Package/strongswan/Default)
  TITLE+= SCEP client
  DEPENDS:= strongswan
endef

define Package/strongswan-scepclient/description
$(call Package/strongswan/description/Default)
 This package contains the SCEP client.
endef

define Package/strongswan-swanctl
$(call Package/strongswan/Default)
  TITLE+= swanctl utility
  DEPENDS:= strongswan +strongswan-mod-vici
endef

define Package/strongswan-swanctl/description
$(call Package/strongswan/description/Default)
 This package contains the swanctl utility.
endef

define Package/strongswan-gencerts
$(call Package/strongswan/Default)
  TITLE+= X.509 certificate generation utility
  DEPENDS:= strongswan +strongswan-pki bash
endef

define Package/strongswan-gencerts/description
$(call Package/strongswan/description/Default)
 This package contains the X.509 certificate generation utility.
endef

define Package/strongswan-libtls
$(call Package/strongswan/Default)
  TITLE+= libtls
  DEPENDS:= strongswan
endef

define Package/strongswan-libtls/description
$(call Package/strongswan/description/Default)
 This package contains libtls for strongSwan plugins eap-tls, eap-ttls,
 eap-peap, tnc-tnccs
endef

define BuildPlugin
  define Package/strongswan-mod-$(1)
    $$(call Package/strongswan/Default)
    TITLE:= StrongSwan $(2) plugin
    DEPENDS:= strongswan $(3)
  endef

  define Package/strongswan-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/etc/strongswan.d/charon
	if [ -f $(PKG_INSTALL_DIR)/etc/strongswan.d/charon/$(1).conf ]; then \
		$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/strongswan.d/charon/$(1).conf $$(1)/etc/strongswan.d/charon/; fi
	$(INSTALL_DIR) $$(1)/usr/lib/ipsec/plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/plugins/libstrongswan-$(1).so \
		$$(1)/usr/lib/ipsec/plugins/
	$(call Plugin/$(1)/install,$$(1))
  endef

  $$(eval $$(call BuildPackage,strongswan-mod-$(1)))
endef

CONFIGURE_ARGS+= \
	--disable-scripts \
	--disable-static \
	--disable-fast \
	--enable-mediation \
	--with-systemdsystemunitdir=no \
	$(if $(CONFIG_PACKAGE_strongswan-charon-cmd),--enable-cmd,--disable-cmd) \
	$(if $(CONFIG_PACKAGE_strongswan-pki),--enable-pki,--disable-pki) \
	$(if $(CONFIG_PACKAGE_strongswan-scepclient),--enable-scepclient,--disable-scepclient) \
	--with-random-device=/dev/random \
	--with-urandom-device=/dev/urandom \
	--with-routing-table="$(call qstrip,$(CONFIG_STRONGSWAN_ROUTING_TABLE))" \
	--with-routing-table-prio="$(call qstrip,$(CONFIG_STRONGSWAN_ROUTING_TABLE_PRIO))" \
	$(foreach m,$(PKG_MOD_AVAILABLE), \
	  $(if $(CONFIG_PACKAGE_strongswan-mod-$(m)),--enable-$(m),--disable-$(m)) \
	) \
	ac_cv_search___atomic_load=no

EXTRA_LDFLAGS+= -Wl,-rpath-link,$(STAGING_DIR)/usr/lib

define Package/strongswan/conffiles
/etc/strongswan.conf
/etc/strongswan.d/
endef

define Package/strongswan/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/strongswan.conf $(1)/etc/
	echo -e "\ninclude /var/ipsec/strongswan.conf" >> $(1)/etc/strongswan.conf
	$(INSTALL_DIR) $(1)/usr/lib/ipsec
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/libstrongswan.so.* $(1)/usr/lib/ipsec/
endef

define Package/strongswan-default/install
	true
endef

define Package/strongswan-full/install
	true
endef

define Package/strongswan-isakmp/install
	true
endef

define Package/strongswan-minimal/install
	true
endef

define Package/strongswan-charon/install
	$(INSTALL_DIR) $(1)/etc/strongswan.d
	$(CP) $(PKG_INSTALL_DIR)/etc/strongswan.d/charon.conf $(1)/etc/strongswan.d
	$(CP) $(PKG_INSTALL_DIR)/etc/strongswan.d/charon-logging.conf $(1)/etc/strongswan.d
	$(INSTALL_DIR) $(1)/usr/lib/ipsec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/ipsec/charon $(1)/usr/lib/ipsec/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/libcharon.so.* $(1)/usr/lib/ipsec/
endef

define Package/strongswan-charon-cmd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/charon-cmd $(1)/usr/sbin/
endef

define Package/strongswan-ipsec/conffiles
/etc/ipsec.d/
/etc/ipsec.conf
/etc/ipsec.secrets
/etc/ipsec.user
endef

define Package/strongswan-ipsec/install
	$(INSTALL_DIR) $(1)/etc/ $(1)/usr/sbin
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/ipsec.conf $(1)/etc/
	echo -e "\ninclude /var/ipsec/ipsec.conf" >> $(1)/etc/ipsec.conf
	$(INSTALL_CONF) ./files/ipsec.secrets $(1)/etc/
	echo -e "\ninclude /var/ipsec/ipsec.secrets" >> $(1)/etc/ipsec.secrets
	$(INSTALL_CONF) ./files/ipsec.user $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ipsec.init $(1)/etc/init.d/ipsec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ipsec $(1)/usr/sbin/
endef

define Package/strongswan-ipsec/postinst
#!/bin/sh

[ -z "$${IPKG_INSTROOT}" ] || exit 0

opkg list-changed-conffiles | grep -qx /etc/ipsec.conf || {
	rm -f /etc/ipsec.conf-opkg
}
endef

define Package/strongswan-libnttfft/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/libnttfft.so.* $(1)/usr/lib/ipsec/
endef

define Package/strongswan-pki/install
	$(INSTALL_DIR) $(1)/etc/strongswan.d
	$(CP) $(PKG_INSTALL_DIR)/etc/strongswan.d/pki.conf $(1)/etc/strongswan.d/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pki $(1)/usr/bin/
endef

define Package/strongswan-scepclient/install
	$(INSTALL_DIR) $(1)/etc/strongswan.d
	$(CP) $(PKG_INSTALL_DIR)/etc/strongswan.d/scepclient.conf $(1)/etc/strongswan.d/
	$(INSTALL_DIR) $(1)/usr/lib/ipsec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/ipsec/scepclient $(1)/usr/lib/ipsec/
endef

define Package/strongswan-swanctl/conffiles
/etc/config/ipsec
/etc/swanctl/
endef

define Package/strongswan-swanctl/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/swanctl/{bliss,conf.d,ecdsa,pkcs{12,8},private,pubkey,rsa}
	$(INSTALL_DIR) $(1)/etc/swanctl/x509{,aa,ac,ca,crl,ocsp}
	$(CP) $(PKG_INSTALL_DIR)/etc/swanctl/swanctl.conf $(1)/etc/swanctl/
	echo "include /var/swanctl/swanctl.conf" >> $(1)/etc/swanctl/swanctl.conf
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/swanctl $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/swanctl.init $(1)/etc/init.d/swanctl
endef

define Package/strongswan-gencerts/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/gencerts.sh $(1)/usr/bin/gencerts
endef

define Package/strongswan-libtls/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/libtls.so.* $(1)/usr/lib/ipsec/
endef

define Plugin/duplicheck/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec/plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/duplicheck $(1)/usr/lib/ipsec/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/plugins/libstrongswan-duplicheck.so $(1)/usr/lib/ipsec/plugins/
endef

define Plugin/eap-radius/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec/plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/libradius.so.* $(1)/usr/lib/ipsec/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/plugins/libstrongswan-eap-radius.so $(1)/usr/lib/ipsec/plugins/
endef

define Plugin/attr-sql/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/pool $(1)/usr/lib/ipsec/
endef

define Plugin/stroke/install
	$(INSTALL_DIR) $(1)/etc/ipsec.d/{aacerts,acerts,cacerts,certs,crls,ocspcerts,private,reqs}

	$(INSTALL_DIR) $(1)/usr/lib/ipsec/plugins
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/ipsec/{starter,stroke} $(1)/usr/lib/ipsec/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/plugins/libstrongswan-stroke.so $(1)/usr/lib/ipsec/plugins/
endef

define Plugin/updown/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec/plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/_updown $(1)/usr/lib/ipsec/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/plugins/libstrongswan-updown.so $(1)/usr/lib/ipsec/plugins/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/ipsec
	$(CP) ./files/etc/hotplug.d/ipsec/01-user \
		$(1)/etc/hotplug.d/ipsec/01-user
endef

define Plugin/vici/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec/plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/libvici.so.* $(1)/usr/lib/ipsec/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/plugins/libstrongswan-vici.so $(1)/usr/lib/ipsec/plugins/
endef

define Plugin/whitelist/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec/plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/whitelist $(1)/usr/lib/ipsec/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/plugins/libstrongswan-whitelist.so $(1)/usr/lib/ipsec/plugins/
endef

define Plugin/kernel-libipsec/install
	$(INSTALL_DIR) $(1)/usr/lib/ipsec
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ipsec/libipsec.so.* $(1)/usr/lib/ipsec/
endef

$(eval $(call BuildPackage,strongswan))
$(eval $(call BuildPackage,strongswan-default))
$(eval $(call BuildPackage,strongswan-full))
$(eval $(call BuildPackage,strongswan-minimal))
$(eval $(call BuildPackage,strongswan-isakmp))
$(eval $(call BuildPackage,strongswan-charon))
$(eval $(call BuildPackage,strongswan-charon-cmd))
$(eval $(call BuildPackage,strongswan-ipsec))
$(eval $(call BuildPackage,strongswan-libnttfft))
$(eval $(call BuildPackage,strongswan-pki))
$(eval $(call BuildPackage,strongswan-scepclient))
$(eval $(call BuildPackage,strongswan-swanctl))
$(eval $(call BuildPackage,strongswan-gencerts))
$(eval $(call BuildPackage,strongswan-libtls))
$(eval $(call BuildPlugin,addrblock,RFC 3779 address block constraint support,))
$(eval $(call BuildPlugin,aes,AES crypto,))
$(eval $(call BuildPlugin,af-alg,AF_ALG crypto interface to Linux Crypto API,+kmod-crypto-user))
$(eval $(call BuildPlugin,agent,SSH agent signing,))
$(eval $(call BuildPlugin,attr,file based config,))
$(eval $(call BuildPlugin,attr-sql,SQL based config,+strongswan-charon))
$(eval $(call BuildPlugin,bliss,BLISS crypto,+strongswan-libnttfft +strongswan-mod-mgf1 +strongswan-mod-hmac))
$(eval $(call BuildPlugin,blowfish,Blowfish crypto,))
$(eval $(call BuildPlugin,ccm,CCM AEAD wrapper crypto,))
$(eval $(call BuildPlugin,chapoly,ChaCha20-Poly1305 AEAD crypto,))
$(eval $(call BuildPlugin,cmac,CMAC crypto,))
$(eval $(call BuildPlugin,connmark,netfilter connection marking,))
$(eval $(call BuildPlugin,constraints,advanced X509 constraint checking,))
$(eval $(call BuildPlugin,coupling,IKEv2 plugin to couple peer certificates permanently to authentication,))
$(eval $(call BuildPlugin,ctr,Counter Mode wrapper crypto,))
$(eval $(call BuildPlugin,curl,cURL fetcher plugin,+PACKAGE_strongswan-mod-curl:libcurl))
$(eval $(call BuildPlugin,curve25519,Curve25519 Diffie-Hellman,))
$(eval $(call BuildPlugin,des,DES crypto,))
$(eval $(call BuildPlugin,dhcp,DHCP based attribute provider,))
$(eval $(call BuildPlugin,dnskey,DNS RR key decoding,))
$(eval $(call BuildPlugin,duplicheck,advanced duplicate checking,))
$(eval $(call BuildPlugin,eap-identity,EAP identity helper,))
$(eval $(call BuildPlugin,eap-md5,EAP MD5 (CHAP) EAP auth,))
$(eval $(call BuildPlugin,eap-mschapv2,EAP MS-CHAPv2 EAP auth,+strongswan-mod-md4 +strongswan-mod-des))
$(eval $(call BuildPlugin,eap-radius,EAP RADIUS auth,))
$(eval $(call BuildPlugin,eap-tls,EAP TLS auth,+strongswan-libtls))
$(eval $(call BuildPlugin,farp,fake arp respsonses,))
$(eval $(call BuildPlugin,fips-prf,FIPS PRF crypto,+strongswan-mod-sha1))
$(eval $(call BuildPlugin,forecast,forward multi/broadcast traffic,+kmod-ipt-conntrack-extra))
$(eval $(call BuildPlugin,gcm,GCM AEAD wrapper crypto,))
$(eval $(call BuildPlugin,gcrypt,libgcrypt,+PACKAGE_strongswan-mod-gcrypt:libgcrypt))
$(eval $(call BuildPlugin,gmp,libgmp,+PACKAGE_strongswan-mod-gmp:libgmp))
$(eval $(call BuildPlugin,gmpdh,DH-Groups; no libgmp dep,))
$(eval $(call BuildPlugin,ha,high availability cluster,))
$(eval $(call BuildPlugin,hmac,HMAC crypto,))
$(eval $(call BuildPlugin,kernel-libipsec,libipsec kernel interface,))
$(eval $(call BuildPlugin,kernel-netlink,netlink kernel interface,))
$(eval $(call BuildPlugin,ldap,LDAP,+PACKAGE_strongswan-mod-ldap:libopenldap))
$(eval $(call BuildPlugin,led,LED blink on IKE activity,))
$(eval $(call BuildPlugin,load-tester,load testing,))
$(eval $(call BuildPlugin,nonce,nonce genereation,))
$(eval $(call BuildPlugin,md4,MD4 crypto,))
$(eval $(call BuildPlugin,md5,MD5 crypto,))
$(eval $(call BuildPlugin,mgf1,MGF1 crypto,))
$(eval $(call BuildPlugin,mysql,MySQL database interface,+strongswan-mod-sql +PACKAGE_strongswan-mod-mysql:libmysqlclient-r))
$(eval $(call BuildPlugin,newhope,New Hope crypto,+strongswan-libnttfft +strongswan-mod-chapoly +strongswan-mod-sha3))
$(eval $(call BuildPlugin,ntru,NTRU crypto,+strongswan-mod-mgf1))
$(eval $(call BuildPlugin,openssl,OpenSSL crypto,+PACKAGE_strongswan-mod-openssl:libopenssl))
$(eval $(call BuildPlugin,pem,PEM decoding,))
$(eval $(call BuildPlugin,pgp,PGP key decoding,))
$(eval $(call BuildPlugin,pkcs1,PKCS1 key decoding,))
$(eval $(call BuildPlugin,pkcs7,PKCS7 key decoding,))
$(eval $(call BuildPlugin,pkcs8,PKCS8 key decoding,))
$(eval $(call BuildPlugin,pkcs11,PKCS11 key decoding,))
$(eval $(call BuildPlugin,pkcs12,PKCS12 key decoding,))
$(eval $(call BuildPlugin,pubkey,raw public key,))
$(eval $(call BuildPlugin,random,RNG,))
$(eval $(call BuildPlugin,rc2,RC2 crypto,))
$(eval $(call BuildPlugin,resolve,DNS resolver,))
$(eval $(call BuildPlugin,revocation,X509 CRL/OCSP revocation,))
$(eval $(call BuildPlugin,sha1,SHA1 crypto,))
$(eval $(call BuildPlugin,sha2,SHA2 crypto,))
$(eval $(call BuildPlugin,sha3,SHA3 and SHAKE crypto,))
$(eval $(call BuildPlugin,smp,SMP configuration and control interface,+PACKAGE_strongswan-mod-smp:libxml2))
$(eval $(call BuildPlugin,socket-default,default socket implementation for charon,))
$(eval $(call BuildPlugin,socket-dynamic,dynamic socket implementation for charon,))
$(eval $(call BuildPlugin,sql,SQL database interface,))
$(eval $(call BuildPlugin,sqlite,SQLite database interface,+strongswan-mod-sql +PACKAGE_strongswan-mod-sqlite:libsqlite3))
$(eval $(call BuildPlugin,sshkey,SSH key decoding,))
$(eval $(call BuildPlugin,stroke,Stroke,+strongswan-charon +strongswan-ipsec))
$(eval $(call BuildPlugin,test-vectors,crypto test vectors,))
$(eval $(call BuildPlugin,uci,UCI config interface,+PACKAGE_strongswan-mod-uci:libuci))
$(eval $(call BuildPlugin,unity,Cisco Unity extension,))
$(eval $(call BuildPlugin,updown,updown firewall,))
$(eval $(call BuildPlugin,vici,Versatile IKE Configuration Interface,))
$(eval $(call BuildPlugin,whitelist,peer identity whitelisting,))
$(eval $(call BuildPlugin,x509,x509 certificate,))
$(eval $(call BuildPlugin,xauth-eap,EAP XAuth backend,))
$(eval $(call BuildPlugin,xauth-generic,generic XAuth backend,))
$(eval $(call BuildPlugin,xcbc,xcbc crypto,))
