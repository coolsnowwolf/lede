#
# Copyright (C) 2017 Lucian Cristian <lucian.cristian@gmail.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uw-imap
PKG_VERSION:=2007f
PKG_RELEASE:=5

PKG_SOURCE:=imap-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
	https://www.mirrorservice.org/sites/ftp.cac.washington.edu/imap \
	http://ftp.ntua.gr/pub/net/mail/imap \
	http://gd.tuwien.ac.at/infosys/mail/imap

PKG_HASH:=53e15a2b5c1bc80161d42e9f69792a3fa18332b7b771910131004eb520004a28
PKG_BUILD_DIR:=$(BUILD_DIR)/imap-$(PKG_VERSION)

PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE.txt

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/uw-imap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=University of Washington IMAP toolkit
  URL:=https://www.washington.edu/imap/
  DEPENDS:=+libpthread +libopenssl
endef

define Package/uw-imap/description
    The UW IMAP tookit includes the following:
    c-lient library : an API (application programming interface) used to build email clients and servers,
    including support for IMAP,POP3, SMTP and NNTP protocols and for local mailbox file access on Unix and Windows
endef

TARGET_CFLAGS += \
	-fPIC -DPIC -D_REENTRANT -DDISABLE_POP_PROXY -fno-strict-aliasing -Wno-pointer-sign \
	-Wno-implicit-function-declaration -Wno-incompatible-pointer-types

MAKE_FLAGS += \
	SSLINCLUDE=$(STAGING_DIR)/usr/include SSLCERTS=/etc/ssl/certs GCCOPTLEVEL=" -Os" \
	SSLDIR=$(STAGING_DIR)/usr SHLIBNAME=libc-client.so.$(PKG_VERSION) $(if $(CONFIG_IPV6),IP=6,IP=4)

define Build/Compile
    $(MAKE) -C $(PKG_BUILD_DIR) slx EXTRACFLAGS='$(TARGET_CFLAGS)' CC='$(TARGET_CC)' $(MAKE_FLAGS)
endef

define Build/InstallDev
	$(INSTALL_DIR)	$(1)/usr/lib \
			$(1)/usr/include/c-client
	$(CP) $(PKG_BUILD_DIR)/c-client/libc-client.so.$(PKG_VERSION) $(1)/usr/lib/
	$(LN) libc-client.so.$(PKG_VERSION) $(1)/usr/lib/libc-client.so
	$(CP) $(PKG_BUILD_DIR)/c-client/linkage.{c,h} $(1)/usr/include/c-client/
	$(CP) $(PKG_BUILD_DIR)/src/c-client/*.h $(1)/usr/include/c-client/
	$(CP) $(PKG_BUILD_DIR)/src/osdep/unix/*.h $(1)/usr/include/c-client/
	$(LN) os_slx.h $(1)/usr/include/c-client/osdep.h
endef

define Package/uw-imap/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/c-client/libc-client.so.$(PKG_VERSION) $(1)/usr/lib/
	$(LN) libc-client.so.$(PKG_VERSION) $(1)/usr/lib/libc-client.so
endef


$(eval $(call BuildPackage,uw-imap))
