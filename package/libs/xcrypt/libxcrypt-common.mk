PKG_SOURCE_NAME:=libxcrypt
PKG_VERSION:=4.4.38
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/besser82/$(PKG_SOURCE_NAME)/releases/download/v$(PKG_VERSION)
PKG_HASH:=80304b9c306ea799327f01d9a7549bdb28317789182631f1b54f4511b4206dd6

PKG_MAINTAINER:=
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING.LIB

define Package/libxcrypt/Default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://github.com/besser82/libxcrypt
  TITLE:=Extended crypt library
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
