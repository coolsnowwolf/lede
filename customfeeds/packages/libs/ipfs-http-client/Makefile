#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ipfs-http-client
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/vasild/cpp-ipfs-http-client.git
PKG_SOURCE_DATE:=2021-03-01
PKG_SOURCE_VERSION:=27f6439303f7afb5b8b2e4154f4269ba296059c7
PKG_MIRROR_HASH:=61814d666bbd927750a0af7d350ad8ba27c2a78e0b5f5196ec4e0b5049ef2690

PKG_MAINTAINER:=Leonid Esman <leonid.esman@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.MIT

PKG_BUILD_DEPENDS:=nlohmannjson

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/ipfs-http-client/Default/description
  IPFS (the InterPlanetary File System) is the Distributed Web.
  Specs, docs, sources, links: https://ipfs.io/ and https://github.com/ipfs.
  This is Vasil Dimov's C++ IPFS HTTP API client library.
endef

define Package/libipfs-http-client
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=IPFS client library
  URL:=https://github.com/vasild/cpp-ipfs-http-client
  DEPENDS:= +libcurl +libstdcpp +libatomic
endef

define Package/libipfs-http-client/description
  $(call Package/ipfs-http-client/Default/description)
  This package contains shared library.
endef

define Package/ipfs-http-client-tests
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=IPFS client library tests
  URL:=https://github.com/vasild/cpp-ipfs-http-client
  DEPENDS:=+libipfs-http-client +libcurl
endef

define Package/ipfs-http-client-tests/description
  $(call Package/ipfs-http-client/Default/description)
  This package contains library tests.
endef

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_TESTING=ON

define Package/libipfs-http-client/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libipfs-http-client.so* $(1)/usr/lib
endef

define Package/ipfs-http-client-tests/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_block		$(1)/usr/bin/ipfs-block
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_config	$(1)/usr/bin/ipfs-config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_dht		$(1)/usr/bin/ipfs-dht
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_files		$(1)/usr/bin/ipfs-files
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_generic	$(1)/usr/bin/ipfs-generic
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_key		$(1)/usr/bin/ipfs-key
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_name		$(1)/usr/bin/ipfs-name
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_object	$(1)/usr/bin/ipfs-object
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_pin		$(1)/usr/bin/ipfs-pin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test/test_swarm		$(1)/usr/bin/ipfs-swarm
endef

$(eval $(call BuildPackage,libipfs-http-client))
$(eval $(call BuildPackage,ipfs-http-client-tests))
