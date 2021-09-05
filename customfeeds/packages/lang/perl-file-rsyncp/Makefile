# Copyright (C) 2018 OpenWrt

include $(TOPDIR)/rules.mk

PKG_NAME:=perl-file-rsyncp
PKG_VERSION:=0.74
PKG_RELEASE:=1

PKG_SOURCE:=File-RsyncP-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://search.cpan.org/CPAN/authors/id/C/CB/CBARRATT/
PKG_HASH:=ba4df5f9b0db6c9d86a6c5cf9861cf00d17b18e77cfa028e7a9157c0015a5aa3
PKG_BUILD_DIR:=$(BUILD_DIR)/perl/File-RsyncP-$(PKG_VERSION)

PKG_MAINTAINER:=Carsten Wolff <carsten@wolffcarsten.de>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE README

include $(INCLUDE_DIR)/package.mk
include ../perl/perlmod.mk

define Package/perl-file-rsyncp
  SUBMENU:=Perl
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Perl Rsync client
  URL:=http://search.cpan.org/~cbarratt/File-RsyncP/
  DEPENDS:=perl +perlbase-autoloader +perlbase-socket +perlbase-getopt +perlbase-data +perlbase-config +perlbase-encode +perlbase-fcntl +perlbase-file
endef

define Build/Configure
	$(call perlmod/Configure,,)
	$(call perlmod/Configure,,,$(PKG_BUILD_DIR)/Digest)
	$(call perlmod/Configure,,,$(PKG_BUILD_DIR)/FileList)
	$(call Build/Configure/Default,,rsync_cv_HAVE_LONGLONG=yes,FileList)
endef

define Build/Compile
	PERL5LIB=$(PERL_LIB) $(MAKE) -C $(PKG_BUILD_DIR)/Digest
	PERL5LIB=$(PERL_LIB) $(MAKE) -C $(PKG_BUILD_DIR)/FileList
	PERL5LIB=$(PERL_LIB) $(MAKE) -C $(PKG_BUILD_DIR)
endef

define Package/perl-file-rsyncp/install
	$(INSTALL_DIR) $(strip $(1))$(PERL_SITELIB)/File/RsyncP
	$(INSTALL_DIR) $(strip $(1))$(PERL_SITELIB)/auto/File/RsyncP/Digest
	$(INSTALL_DIR) $(strip $(1))$(PERL_SITELIB)/auto/File/RsyncP/FileList
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lib/File/RsyncP.pm $(strip $(1))$(PERL_SITELIB)/File
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lib/File/RsyncP/FileIO.pm $(strip $(1))$(PERL_SITELIB)/File/RsyncP
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/Digest/Digest.pm $(strip $(1))$(PERL_SITELIB)/File/RsyncP
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/FileList/FileList.pm $(strip $(1))$(PERL_SITELIB)/File/RsyncP
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/Digest/blib/lib/auto/File/RsyncP/Digest/autosplit.ix $(strip $(1))$(PERL_SITELIB)/auto/File/RsyncP/Digest
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/Digest/blib/arch/auto/File/RsyncP/Digest/Digest.so $(strip $(1))$(PERL_SITELIB)/auto/File/RsyncP/Digest
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/FileList/blib/lib/auto/File/RsyncP/FileList/autosplit.ix $(strip $(1))$(PERL_SITELIB)/auto/File/RsyncP/FileList
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/FileList/blib/arch/auto/File/RsyncP/FileList/FileList.so $(strip $(1))$(PERL_SITELIB)/auto/File/RsyncP/FileList
endef

$(eval $(call BuildPackage,perl-file-rsyncp))
