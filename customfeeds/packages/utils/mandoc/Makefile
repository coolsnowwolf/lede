include $(TOPDIR)/rules.mk

PKG_NAME:=mandoc
PKG_VERSION:=1.14.5
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://mandoc.bsd.lv/snapshots/
PKG_HASH:=8219b42cb56fc07b2aa660574e6211ac38eefdbf21f41b698d3348793ba5d8f7

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=BSD-3-Clause BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mandoc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=mandoc document formatting system
  DEPENDS:=+zlib
  URL:=https://mandoc.bsd.lv/
endef

define Package/sed/description
mandoc is a suite of tools compiling mdoc, the roff macro language
of choice for BSD manual pages, and man, the predominant historical
language for UNIX manuals. It is small, ISO C, ISC-licensed, and quite
fast. The main component of the toolset is the mandoc utility program,
based on the libmandoc validating compiler, to format output for UTF-8
and ASCII UNIX terminals, HTML 5, PostScript, and PDF.
endef

CONFIGURE_VARS=\
	CC=$(CC) \
	HAVE_FTS=0 \
	HAVE_REALLOCARRAY=0 \

define Package/mandoc/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mandoc $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mandoc))
