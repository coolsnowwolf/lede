#
# Copyright (C) 2011-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=opensc
PKG_VERSION:=0.21.0
PKG_RELEASE:=$(AUTORELEASE)
PKG_HASH:=2bfbbb1dcb4b8d8d75685a3e95c30798fb6411d4efab3690fd89d2cb25f3325e

PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_SOURCE_URL:=https://github.com/OpenSC/OpenSC/releases/download/$(PKG_VERSION)/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_BUILD_DEPENDS:=pcsc-lite
PKG_FIXUP:=libtool

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libopensc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=OpenSC libraries for smart cards
  URL:=https://github.com/OpenSC/OpenSC/wiki
  DEPENDS:=+libopenssl +libpthread +zlib
  MENU:=1
endef

define Package/libopensc/description
  OpenSC provides a set of libraries and utilities to work with smart cards.
  Its main focus is on cards that support cryptographic operations, and
  facilitate their use in security applications such as authentication,
  mail encryption and digital signatures.
endef

define Package/libopensc-pkcs11
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=OpenSC - PKCS11 provider
  URL:=https://github.com/OpenSC/OpenSC/wiki
  DEPENDS:=libopensc
endef

define Package/libopensc-pkcs11/description
  OpenSC PKCS#11 provider
endef

define Package/libpkcs11-spy
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=PKCS11 spying wrapper
  URL:=https://github.com/OpenSC/OpenSC/wiki
  DEPENDS:=+libopenssl +libpthread
endef

define Package/libpkcs11-spy/dscription
  PKCS#11 spying wrapper
endef

define Package/opensc-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=OpenSC - tools for smart cards
  URL:=https://github.com/OpenSC/OpenSC/wiki
  DEPENDS:=+libopensc
  MENU:=1
endef

define Package/opensc-utils/description
  OpenSC utilities
endef

define ToolGen
define Package/opensc-utils-$(subst _,-,$(firstword $(subst :, ,$(1))))
  TITLE:=$(firstword $(subst :, ,$(1))) utility from opensc
  URL:=https://github.com/OpenSC/OpenSC/wiki
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=opensc-utils $(wordlist 2,$(words $(subst :, ,$(1))),$(subst :, ,$(1)))
endef
endef

define ProfileGen
define Package/libopensc-profile-$(subst _,-,$(firstword $(subst :, ,$(1))))
  TITLE:=$(firstword $(subst :, ,$(1))) card profile for opensc
  URL:=https://github.com/OpenSC/OpenSC/wiki
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=libopensc
endef
endef

CONFIGURE_ARGS += \
	--disable-notify \
	--disable-strict

TOOLS:= \
	cardos-tool \
	cryptoflex-tool \
	dnie-tool \
	eidenv \
	iasecc-tool \
	netkey-tool \
	openpgp-tool \
	opensc-tool \
	opensc-explorer:+libncurses:+libreadline \
	piv-tool \
	pkcs11-tool \
	pkcs15-crypt \
	pkcs15-init \
	pkcs15-tool \
	sc-hsm-tool \
	westcos-tool

PROFILES:= \
	asepcos \
	authentic \
	cardos \
	cyberflex \
	entersafe \
	epass2003 \
	flex \
	gpk \
	ias_adele_admin1 \
	ias_adele_admin2 \
	ias_adele_common \
	iasecc_admin_eid \
	iasecc_generic_oberthur \
	iasecc_generic_pki \
	iasecc \
	incrypto34 \
	jcop \
	miocos \
	muscle \
	myeid \
	oberthur \
	openpgp \
	pkcs15 \
	rutoken_ecp \
	rutoken \
	sc-hsm \
	setcos \
	starcos \
	westcos

$(foreach file,$(TOOLS),$(eval $(call ToolGen,$(file))))
$(foreach file,$(PROFILES),$(eval $(call ProfileGen,$(file))))

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopensc.{la,so}* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsmm-local.{la,so}* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/opensc-pkcs11.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkcs11-spy.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkcs11
	$(LN) ../pkcs11-spy.so $(1)/usr/lib/pkcs11/
	$(LN) ../opensc-pkcs11.so $(1)/usr/lib/pkcs11/
	$(INSTALL_DIR) $(1)/usr/share/opensc
	$(CP) $(PKG_INSTALL_DIR)/usr/share/opensc/* $(1)/usr/share/opensc/
endef

define Package/libopensc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libopensc.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsmm-local.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/opensc.conf $(1)/etc/
endef

define Package/libopensc-pkcs11/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/opensc-pkcs11.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkcs11
	$(LN) ../opensc-pkcs11.so $(1)/usr/lib/pkcs11/
endef

define Package/libpkcs11-spy/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkcs11-spy.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkcs11
	$(LN) ../pkcs11-spy.so $(1)/usr/lib/pkcs11/
endef

define Package/opensc-card-profiles
	$(INSTALL_DIR) $(1)/usr/share/opensc
	$(CP) $(PKG_INSTALL_DIR)/usr/share/opensc/* $(1)/usr/share/opensc/
endef

define Package/opensc-utils/install
	true
endef

define ToolInstall
define Package/opensc-utils-$(subst _,-,$(firstword $(subst :, ,$(1))))/install
	$(INSTALL_DIR) $$(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/$(firstword $(subst :, ,$(1))) \
		$$(1)/usr/bin/
endef
endef

define ProfileInstall
define Package/libopensc-profile-$(subst _,-,$(firstword $(subst :, ,$(1))))/install
	$(INSTALL_DIR) $$(1)/usr/share/opensc
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/share/opensc/$(firstword $(subst :, ,$(1))).profile \
		$$(1)/usr/share/opensc
endef
endef

$(foreach file,$(TOOLS),$(eval $(call ToolInstall,$(file))))
$(foreach file,$(PROFILES),$(eval $(call ProfileInstall,$(file))))

$(eval $(call BuildPackage,libopensc))
$(eval $(call BuildPackage,libopensc-pkcs11))
$(eval $(call BuildPackage,libpkcs11-spy))

$(eval $(call BuildPackage,opensc-utils))
$(foreach file,$(TOOLS),$(eval $(call BuildPackage,opensc-utils-$(subst _,-,$(firstword $(subst :, ,$(file)))))))
$(foreach file,$(PROFILES),$(eval $(call BuildPackage,libopensc-profile-$(subst _,-,$(firstword $(subst :, ,$(file)))))))
