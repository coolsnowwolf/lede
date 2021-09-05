# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-file-next
PKG_VERSION:=1.18
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/P/PE/PETDANCE/
PKG_SOURCE:=File-Next-$(PKG_VERSION).tar.gz
PKG_HASH:=f900cb39505eb6e168a9ca51a10b73f1bbde1914b923a09ecd72d9c02e6ec2ef

PKG_LICENSE:=Artistic-2.0
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/File-Next-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-file-next
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=File finding module
  URL:=http://search.cpan.org/dist/File-Next/
  DEPENDS:=perl +perlbase-file
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-file-next/install
	$(call perlmod/Install,$(1),File auto/File)
endef

$(eval $(call BuildPackage,perl-file-next))
