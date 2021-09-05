#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gnupg
PKG_VERSION:=1.4.23
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://gnupg.org/ftp/gcrypt/gnupg
PKG_HASH:=c9462f17e651b6507848c08c430c791287cd75491f8b5a8b50c6ed46b12678ba

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/gnupg/Default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  DEPENDS:=+zlib +libncurses +libreadline
  TITLE:=GNU privacy guard - a free PGP replacement
  URL:=http://www.gnupg.org/
endef

define Package/gnupg
  $(call Package/gnupg/Default)
  MENU:=1
  ALTERNATIVES:=200:/usr/bin/gpg:/usr/bin/gpg1
endef

define Package/gpgv
  $(call Package/gnupg/Default)
  TITLE:=GnuPG signature verification only
  ALTERNATIVES:=200:/usr/bin/gpgv:/usr/bin/gpgv1
endef

define Package/gnupg-utils
  $(call Package/gnupg/Default)
  DEPENDS:=gnupg +libcurl
  TITLE:=Key management utilities for GnuPG
endef

define Package/gnupg/description
 GnuPG is GNU's tool for secure communication and data storage.
 It can be used to encrypt data and to create digital signatures.
 It includes an advanced key management facility and is compliant
 with the proposed OpenPGP Internet standard as described in RFC2440.
 .
 GnuPG does not use any patented algorithms so it cannot be compatible
 with PGP2 because it uses IDEA (which is patented worldwide).
endef

define Package/gpgv/description
 GPGv is a stripped down version of GnuPG that only checks signatures.
 .
 GnuPG does not use any patented algorithms so it cannot be compatible
 with PGP2 because it uses IDEA (which is patented worldwide).
endef

define Package/gnupg-utils/description
 Key management utilies for GnuPG.
 This package is needed to import keys from a keyserver.
endef

CONFIGURE_ARGS += \
	--disable-rpath \
	--disable-asm \
	--disable-gnupg-iconv \
	--disable-card-support \
	--disable-agent-support \
	--disable-bzip2 \
	--disable-ldap \
	--disable-finger \
	--disable-dns-srv \
	--disable-regex

MAKE_FLAGS += \
	SUBDIRS="m4 intl zlib util mpi cipher tools g10 keyserver ${checks}" \

TARGET_CFLAGS += $(if $(CONFIG_GCC_USE_VERSION_10),-DEXTERN_UNLESS_MAIN_MODULE=static)

define Package/gnupg/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gpg $(1)/usr/bin/gpg1
endef

define Package/gpgv/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gpgv $(1)/usr/bin/gpgv1
endef

define Package/gnupg-utils/install
	$(INSTALL_DIR) $(1)/usr/lib/gnupg
	for file in gpgkeys_curl gpgkeys_hkp; do \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/gnupg/$$$$file \
		  $(1)/usr/lib/gnupg/; \
	done
endef

$(eval $(call BuildPackage,gnupg))
$(eval $(call BuildPackage,gpgv))
$(eval $(call BuildPackage,gnupg-utils))
