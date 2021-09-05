include $(TOPDIR)/rules.mk

PKG_NAME:=libxcrypt
PKG_VERSION:=4.4.17
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/besser82/libxcrypt/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=7665168d0409574a03f7b484682e68334764c29c21ca5df438955a381384ca07

PKG_MAINTAINER:=
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING.LIB

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libxcrypt
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://github.com/besser82/libxcrypt
  TITLE:=Extended crypt library
  BUILDONLY:=1
endef

define Package/libxcrypt/description
  libxcrypt is a modern library for one-way hashing of passwords. It supports
  a wide variety of both modern and historical hashing methods: yescrypt,
  gost-yescrypt, scrypt, bcrypt, sha512crypt, sha256crypt, md5crypt, SunMD5,
  sha1crypt, NT, bsdicrypt, bigcrypt, and descrypt. It provides the traditional
  Unix crypt and crypt_r interfaces, as well as a set of extended interfaces
  pioneered by Openwall Linux, crypt_rn, crypt_ra, crypt_gensalt,
  crypt_gensalt_rn, and crypt_gensalt_ra.
endef

CONFIGURE_ARGS += \
	--disable-shared \
	--disable-failure-tokens \
	--disable-xcrypt-compat-files \
	--disable-obsolete-api \
	--enable-hashes=solaris

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/libxcrypt
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcrypt.{a,la} $(1)/usr/lib/libxcrypt
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*crypt.pc $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,libxcrypt))
