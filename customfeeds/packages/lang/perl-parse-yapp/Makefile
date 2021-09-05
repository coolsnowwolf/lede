#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-parse-yapp
PKG_VERSION:=1.21
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://www.cpan.org/authors/id/W/WB/WBRASWELL/
PKG_SOURCE:=Parse-Yapp-$(PKG_VERSION).tar.gz
PKG_HASH:=3810e998308fba2e0f4f26043035032b027ce51ce5c8a52a8b8e340ca65f13e5

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Andy Walsh <andy.walsh44+github@gmail.com>

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/perl/Parse-Yapp-$(PKG_VERSION)
HOST_BUILD_DEPENDS:=perl/host
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Parse-Yapp-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../perl/perlmod.mk

define Package/perl-parse-yapp
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Yet Another Parser Parser For Perl
  URL:=http://search.cpan.org/dist/Parse-Yapp/
  DEPENDS:=perl +perlbase-essential +perlbase-test
endef

define Host/Configure
        $(call perlmod/host/Configure,,,)
endef

define Host/Compile
        $(call perlmod/host/Compile,,)
endef

define Host/Install
        $(call perlmod/host/Install,$(1),)
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-parse-yapp/install
        $(call perlmod/Install,$(1),Parse auto/Parse)
endef


$(eval $(call BuildPackage,perl-parse-yapp))
$(eval $(call HostBuild))
