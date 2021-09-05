include $(TOPDIR)/rules.mk

PKG_NAME:=whois
PKG_VERSION:=5.5.9
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/w/whois
PKG_HASH:=69088241ed33d2204f153c8005b312a69b60a1429075ff49f42f9f1f73a19c19

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DEPENDS:=perl/host

PKG_MAINTAINER:=Paul Spooren <mail@aparcar.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/whois
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Intelligent WHOIS client
  URL:=https://github.com/rfc1036/whois
endef

define Package/whois/description
This client is intelligent and can automatically select the appropriate whois
server for most queries.

The internal database is often more accurate than IANA's published one, but
please send me any information you have regarding domains and network resources
which are not correctly handled by the program.
endef

EXTRA_CFLAGS+=-DHAVE_GETOPT_LONG
MAKE_FLAGS += PERL=$(STAGING_DIR_HOSTPKG)/usr/bin/perl

define Package/whois/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/whois $(1)/usr/bin/whois
endef

$(eval $(call BuildPackage,whois))
