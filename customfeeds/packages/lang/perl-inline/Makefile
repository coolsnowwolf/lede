#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-inline
PKG_VERSION:=0.86
PKG_RELEASE:=1

PKG_SOURCE:=Inline-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://cpan.metacpan.org/authors/id/I/IN/INGY
PKG_HASH:=510a7de2d011b0db80b0874e8c0f7390010991000ae135cff7474df1e6d51e3a

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/Inline-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/perl/Inline-$(PKG_VERSION)

HOST_BUILD_DEPENDS:=perl/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../perl/perlmod.mk

define Package/perl-inline
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Write subroutines in other languages
  URL:=https://search.cpan.org/dist/Inline/
  DEPENDS:=perl +perlbase-base +perlbase-config +perlbase-cwd +perlbase-digest +perlbase-essential +perlbase-fcntl +perlbase-file
endef

define Build/Configure
        $(call perlmod/Configure,,)
endef

define Build/Compile
        $(call perlmod/Compile,,)
endef

define Package/perl-inline/install
        $(call perlmod/Install,$(1),Inline.pm Inline auto/Inline)
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


$(eval $(call BuildPackage,perl-inline))
$(eval $(call HostBuild))
