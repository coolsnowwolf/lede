#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-xml-parser
PKG_VERSION:=2.46
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://www.cpan.org/authors/id/T/TO/TODDR/
PKG_SOURCE:=XML-Parser-$(PKG_VERSION).tar.gz
PKG_HASH:=d331332491c51cccfb4cb94ffc44f9cd73378e618498d4a37df9e043661c515d

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-1.0-or-later Artistic-1.0-Perl

PKG_BUILD_DIR:=$(BUILD_DIR)/perl/XML-Parser-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/perl/XML-Parser-$(PKG_VERSION)

HOST_BUILD_DEPENDS:=perl/host expat/host

EXPAT_LIBPATH:=$(STAGING_DIR)/usr/lib
EXPAT_INCPATH:=$(STAGING_DIR)/usr/include
HOST_EXPAT_LIBPATH:=$(STAGING_DIR_HOSTPKG)/lib
HOST_EXPAT_INCPATH:=$(STAGING_DIR_HOSTPKG)/include

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../perl/perlmod.mk

define Package/perl-xml-parser
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Perl XML Parser
  URL:=http://search.cpan.org/dist/XML-Parser/
  DEPENDS:=perl +libexpat +perlbase-dynaloader +perlbase-essential
endef

define FixupExpat
	# Manually reconfigure XML::Parser::Expat, as some overrides we provide will not have any effect otherwise
	# Force usage of OpenWRT's expat
	sed \
		-e 's!%%EXPAT_LIBPATH%%!$(2)!g' \
		-e 's!%%EXPAT_INCPATH%%!$(3)!g' \
		-i $(1)/Expat/Makefile.PL
endef

define Build/Configure
	$(call perlmod/Configure,EXPATLIBPATH=$(EXPAT_LIBPATH) EXPATINCPATH=$(EXPAT_INCPATH),,)

	$(call FixupExpat,$(PKG_BUILD_DIR),$(EXPAT_LIBPATH),$(EXPAT_INCPATH))
	$(call perlmod/Configure,,,$(PKG_BUILD_DIR)/Expat)
endef

define Build/Compile
	$(call perlmod/Compile,,)
	$(call perlmod/Compile,,,$(PKG_BUILD_DIR)/Expat)
endef

define Host/Configure
	$(call perlmod/host/Configure,EXPATLIBPATH=$(HOST_EXPAT_LIBPATH) EXPATINCPATH=$(HOST_EXPAT_INCPATH),,)
	$(call FixupExpat,$(HOST_BUILD_DIR),$(HOST_EXPAT_LIBPATH),$(HOST_EXPAT_INCPATH))
	$(call perlmod/host/Configure,,,$(HOST_BUILD_DIR)/Expat)
endef

define Host/Compile
	$(call perlmod/host/Compile,,)
	$(call perlmod/host/Compile,,,$(HOST_BUILD_DIR)/Expat)
endef

define Host/Install
	$(call perlmod/host/Install,$(1),)
endef

define Package/perl-xml-parser/install
	$(call perlmod/Install,$(1),auto/XML XML)
endef


$(eval $(call BuildPackage,perl-xml-parser))
$(eval $(call HostBuild))
