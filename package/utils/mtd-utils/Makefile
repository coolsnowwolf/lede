#
# Copyright (C) 2009-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
PKG_VERSION:=1.5.2
PKG_RELEASE:=1

PKG_SOURCE_URL:=git://git.infradead.org/mtd-utils.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=aea36417067dade75192bafa03af70b6eb2677b1
PKG_MIRROR_HASH:=e11b342b85a36b2e438a8412ec52f87621d3046aec1a93039f8c72de9990b2a7

PKG_INSTALL:=1

PKG_FLAGS:=nonshared

PKG_BUILD_DEPENDS:=util-linux lzo zlib

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=John Crispin <john@phrozen.org>

include $(INCLUDE_DIR)/package.mk

define Package/mtd-utils/Default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://www.linux-mtd.infradead.org/
  DEPENDS:=@NAND_SUPPORT
endef

define Package/ubi-utils
 $(call Package/mtd-utils/Default)
  TITLE:=Utilities for ubi info/debug
endef

define Package/ubi-utils/description
  Utilities for manipulating memory technology devices.
endef

define Package/nand-utils
 $(call Package/mtd-utils/Default)
  TITLE:=Utilities for nand flash read/write/test
endef

define Package/nand-utils/description
  Utilities for NAND devices.
endef

MAKE_FLAGS += \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	BUILDDIR="$(PKG_BUILD_DIR)" \
	LDLIBS+="$(LIBGCC_S)" \
	WITHOUT_XATTR=1 \
	WITHOUT_LZO=1

define Package/ubi-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/{ubiattach,ubicrc32,ubiblock,ubidetach,ubiformat,ubimkvol} $(1)/usr/sbin/
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/{ubinfo,ubinize,ubirename,ubirmvol,ubirsvol,ubiupdatevol} $(1)/usr/sbin/
endef

define Package/nand-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) \
	$(PKG_INSTALL_DIR)/usr/sbin/{nanddump,nandwrite,nandtest,mtdinfo} $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,ubi-utils))
$(eval $(call BuildPackage,nand-utils))
