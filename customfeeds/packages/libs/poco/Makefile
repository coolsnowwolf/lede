#
# Copyright (C) 2007-2016 OpenWrt.org
# Copyright (C) 2017 Daniel Engberg <daniel.engberg.lists@pyret.net>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=poco
PKG_VERSION:=1.10.1
PKG_RELEASE:=$(AUTORELEASE)

ifeq ($(BUILD_VARIANT),all)
_PKG_VERSION:=${PKG_VERSION}-all
PKG_HASH:=2cde4b50778013ab3b7a522aa59bccaa7e85a8ccfc654a354c4d9611b6ce1758
else
_PKG_VERSION:=${PKG_VERSION}
PKG_HASH:=cdab379d7d0394a763821d058eee1e7d4d8214a3caec05c775b60962b2f20762
endif

PKG_SOURCE:=$(PKG_NAME)-$(_PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://pocoproject.org/releases/$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Jean-Michel Julien <jean-michel.julien@trilliantinc.com>
PKG_LICENSE:=BSL-1.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:pocoproject:poco

PKG_BUILD_DEPENDS:=postgresql
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(_PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/poco
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Poco C++ libraries
  URL:=https://www.pocoproject.org/
  DEPENDS:=+libstdcpp +libpthread +librt @!arc
  VARIANT:=minimal
endef

define Package/poco/description
  POrtable COmponents, a modern and powerful open source C++ class libraries
  and frameworks for building network and internet-based applications that
  run on desktop, server and embedded systems.
endef

define Package/poco-all
  $(call Package/poco)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+=(Complete Edition)
  DEPENDS+=+libopenssl
  VARIANT:=all
endef

define Package/poco-all/description
  POrtable COmponents, a modern and powerful open source C++ class libraries
  and frameworks for building network and internet-based applications that
  run on desktop, server and embedded systems. The Complete Edition contains
  all libraries.
endef

CONFIGURE_ARGS += \
	--config=Linux \
	--no-tests \
	--no-samples \
	--no-fpenvironment \
	--no-sharedmemory \
	--no-wstring \
	--shared

ifeq ($(BUILD_VARIANT),all)
	CONFIGURE_ARGS += \
		--typical
	POCO_LIBS={Foundation,XML,JSON,Net,Util,Crypto,NetSSL,Encodings}
else
	CONFIGURE_ARGS += \
		--poquito \
		--minimal
	POCO_LIBS={Foundation,XML,JSON,Net,Util}
endif

define Package/poco/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libPoco$(POCO_LIBS).so* $(1)/usr/lib/
endef

define Package/poco-all/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libPoco$(POCO_LIBS).so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/Poco $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libPoco$(POCO_LIBS).so* $(1)/usr/lib/
endef


$(eval $(call BuildPackage,poco))
$(eval $(call BuildPackage,poco-all))
