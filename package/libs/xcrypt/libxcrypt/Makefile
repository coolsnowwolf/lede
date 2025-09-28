include $(TOPDIR)/rules.mk
include ../libxcrypt-common.mk

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/libxcrypt
$(Package/libxcrypt/Default)
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
	--with-pic \
	--enable-year2038 \
	--disable-xcrypt-compat-files \
	--disable-shared \
	--disable-failure-tokens \
	--disable-obsolete-api \
	--enable-hashes=solaris

define Package/libxcrypt/install
	true
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/libxcrypt
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcrypt.{a,la} $(1)/usr/lib/libxcrypt
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libxcrypt.pc $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,libxcrypt))
