#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-file-sharedir-install
PKG_VERSION:=0.13
PKG_RELEASE:=1

PKG_SOURCE:=File-ShareDir-Install-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.cpan.org/authors/id/E/ET/ETHER
PKG_HASH:=45befdf0d95cbefe7c25a1daf293d85f780d6d2576146546e6828aad26e580f9

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/perl/File-ShareDir-Install-$(PKG_VERSION)
HOST_BUILD_DEPENDS:=perl/host
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/File-ShareDir-Install-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../perl/perlmod.mk

define Package/perl-file-sharedir-install
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Install read-only data files
  URL:=https://search.cpan.org/dist/File-ShareDir-Install/
  DEPENDS:=perl +perlbase-essential +perlbase-extutils +perlbase-file +perlbase-io
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

define Package/perl-file-sharedir-install/install
        $(call perlmod/Install,$(1),File auto/File)
endef


$(eval $(call BuildPackage,perl-file-sharedir-install))
$(eval $(call HostBuild))
