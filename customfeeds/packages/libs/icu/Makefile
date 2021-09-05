#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=icu4c
MAJOR_VERSION:=68
MINOR_VERSION:=2
PKG_VERSION:=$(MAJOR_VERSION).$(MINOR_VERSION)
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(MAJOR_VERSION)_$(MINOR_VERSION)-src.tgz
PKG_SOURCE_URL:=https://github.com/unicode-org/icu/releases/download/release-$(MAJOR_VERSION)-$(MINOR_VERSION)
PKG_HASH:=c79193dee3907a2199b8296a93b52c5cb74332c26f3d167269487680d479d625

PKG_LICENSE:=ICU
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:icu-project:international_components_for_unicode

PKG_MAINTAINER:=Hirokazu MORIKAWA <morikw2@gmail.com>

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_BUILD_DEPENDS:=icu/host
HOST_BUILD_DEPENDS:=python3/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

TAR_OPTIONS+= icu/source --strip-components 2
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

define Package/icu
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=International Components for Unicode
  URL:=http://icu-project.org
  DEPENDS:=+libstdcpp +libpthread
  ABI_VERSION:=$(MAJOR_VERSION)
endef

define Package/icu/description
  ICU is a mature, widely used set of C/C++ and Java libraries providing Unicode and Globalization support for software applications. ICU is widely portable and gives applications the same results on all platforms and between C/C++ and Java software.
  This package supports C/C++.
endef

define Package/icu-full-data
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Full ICU Data
  URL:=http://icu-project.org
  DEPENDS:=+icu
  ABI_VERSION:=$(MAJOR_VERSION)
endef

define Package/icu-full-data/description
  ICU makes use of a wide variety of data tables to provide many of its services. Examples include converter mapping tables, collation rules, transliteration rules, break iterator rules and dictionaries, and other locale data.
  This package contains the complete data library provided by ICU.
  A custom data library can be generated at http://apps.icu-project.org/datacustom/
endef

define Package/icu-data-tools
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=ICU Data manipulation tools
  URL:=http://icu-project.org
  DEPENDS:=+icu
endef

define Package/icu-data-tools/description
 This package provides tools for manipulating ICU data.
endef

CONFIGURE_CMD:= ./runConfigureICU
CONFIGURE_ARGS:= \
	Linux/gcc \
	CC="$(TARGET_CC)" \
	CXX="$(TARGET_CXX)" \
	--target=$(GNU_TARGET_NAME) \
	--host=$(GNU_TARGET_NAME) \
	--build=$(GNU_HOST_NAME) \
	--disable-debug \
	--enable-release \
	--enable-shared \
	--enable-static \
	--enable-draft \
	--enable-renaming \
	--disable-tracing \
	--disable-extras \
	--enable-dyload \
	--with-data-packaging=archive \
	--disable-tests \
	--disable-samples \
	--with-cross-build="$(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)" \
	--prefix=/usr

HOST_CONFIGURE_CMD:= ./runConfigureICU
HOST_CONFIGURE_VARS:=
ifeq ($(HOST_OS),Darwin)
HOST_CONFIGURE_ARGS:= MacOSX
else
HOST_CONFIGURE_ARGS:= Linux/gcc 
endif
HOST_CONFIGURE_ARGS+= \
	--disable-debug \
	--enable-release \
	--enable-shared \
	--enable-static \
	--enable-draft \
	--enable-renaming \
	--disable-tracing \
	--disable-extras \
	--enable-dyload \
	--prefix=$(STAGING_DIR_HOSTPKG)

define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)/data/out
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(2)/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/icu-config $(1)/usr/bin/
	$(SED) 's,^\(prefix\|execprefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/icu-config
	$(LN) $(STAGING_DIR)/usr/bin/icu-config $(2)/bin/
endef

define Host/Install
	$(INSTALL_DIR)  $(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)/config
	$(INSTALL_DIR)  $(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)/bin
	$(INSTALL_DIR)  $(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)/lib
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/config/icucross.* $(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)/config/
	$(INSTALL_BIN)  $(HOST_BUILD_DIR)/bin/icupkg $(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)/bin/
	$(INSTALL_BIN)  $(HOST_BUILD_DIR)/bin/pkgdata $(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)/bin/
ifeq ($(HOST_OS),Darwin)
	$(CP)           $(HOST_BUILD_DIR)/lib/*.dylib* $(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)/lib/
else
	$(CP)           $(HOST_BUILD_DIR)/lib/*.so* $(STAGING_DIR_HOSTPKG)/share/icu/$(PKG_VERSION)/lib/
endif
	$(RM)           $(STAGING_DIR_HOSTPKG)/share/icu/current
	(cd $(STAGING_DIR_HOSTPKG)/share/icu;$(LN) $(PKG_VERSION) current)
endef

define Package/icu/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Package/icu-full-data/install
	$(INSTALL_DIR)  $(1)/usr/share/icu/$(PKG_VERSION)
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/icu/$(PKG_VERSION)/icudt*.dat \
		$(1)/usr/share/icu/$(PKG_VERSION)/
endef

define Package/icu-data-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,icu))
$(eval $(call BuildPackage,icu-full-data))
$(eval $(call BuildPackage,icu-data-tools))
