#
# Copyright (C) 2014 - 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-html-parser
PKG_VERSION:=3.75
PKG_RELEASE:=1

PKG_SOURCE:=HTML-Parser-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.cpan.org/authors/id/C/CA/CAPOEIRAB
PKG_HASH:=ac6b5e25a8df7af54885201e91c45fb9ab6744c08cedc1a38fcc7d95d21193a9

PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl
PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_CPE_ID:=cpe:/a:derrick_oswald:html-parser

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/HTML-Parser-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/perl/HTML-Parser-$(PKG_VERSION)
HOST_BUILD_DEPENDS:=perl/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../perl/perlmod.mk

define Package/perl-html-parser
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=A collection of modules that parse HTML text documents
  URL:=http://search.cpan.org/dist/HTML-Parser/
  DEPENDS:=perl +perl-html-tagset +perlbase-essential +perlbase-xsloader
endef

define Build/Configure
	$(call perlmod/Configure,,)
endef

define Build/Compile
	$(call perlmod/Compile,,)
endef

define Package/perl-html-parser/install
	$(call perlmod/Install,$(1),HTML auto/HTML)
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

$(eval $(call BuildPackage,perl-html-parser))
$(eval $(call HostBuild))
