#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sysrepo
PKG_VERSION:=1.4.122
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/sysrepo/sysrepo/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=2cc7537a03f48dc3c955436e1e0ed077bc3b31a755d6979d24ca42e1187fce01

PKG_MAINTAINER:=Jakov Smolic <jakov.smolic@sartura.hr>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1
PKG_BUILD_DEPENDS:=swig/host
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include ../../lang/python/python3-package.mk

define Package/libsysrepo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=YANG-based data store library
  URL:=https://www.sysrepo.org/
  DEPENDS:=+libyang +libatomic +libprotobuf-c +libev +libredblack +librt +libpthread
endef

define Package/python3-sysrepo
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=YANG-based data store library - Python 3 bindings
  URL:=https://www.sysrepo.org/
  DEPENDS:=+libsysrepo +libstdcpp +python3-base +libyang-cpp
endef

define Package/sysrepo
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=YANG-based data store daemon and plugin
  URL:=https://www.sysrepo.org/
  DEPENDS:=+libsysrepo
endef

define Package/sysrepoctl
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=sysrepo cli tool
  URL:=https://www.sysrepo.org/
  DEPENDS:=+libsysrepo
endef

define Package/sysrepocfg
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=sysrepo configuration tool
  URL:=https://www.sysrepo.org/
  DEPENDS:=+libsysrepo
endef

define Package/sysrepo/description
Sysrepo is an YANG-based configuration and operational state data store for Unix/Linux applications.
endef

SWIG_VERSION:=4.0.1

CMAKE_OPTIONS += \
	-DENABLE_TESTS:BOOL=FALSE \
	-DBUILD_EXAMPLES:BOOL=FALSE \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_BUILD_TYPE="Package" \
	-DREPOSITORY_LOC:PATH=/etc/sysrepo \
	-DCMAKE_DISABLE_FIND_PACKAGE_SWIG=FALSE \
	-DSWIG_EXECUTABLE=$(STAGING_DIR_HOSTPKG)/bin/swig \
	-DCALL_TARGET_BINS_DIRECTLY=OFF \
	-DGEN_LANGUAGE_BINDINGS:BOOL=TRUE \
	-DGEN_PYTHON_BINDINGS:BOOL=TRUE

define Package/libsysrepo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsysrepo.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/etc/sysrepo/yang
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/modules/ietf-netconf-with-defaults.yang $(1)/etc/sysrepo/yang/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/modules/ietf-netconf-notifications.yang $(1)/etc/sysrepo/yang/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/modules/ietf-netconf.yang $(1)/etc/sysrepo/yang/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/modules/ietf-origin.yang $(1)/etc/sysrepo/yang/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/modules/sysrepo.yang $(1)/etc/sysrepo/yang/

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/libsysrepo.default $(1)/etc/uci-defaults/95_libsysrepo
endef

define Package/python3-sysrepo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsysrepo-cpp.so* $(1)/usr/lib
	$(INSTALL_DIR) $(1)$(PYTHON3_PKG_DIR)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/bindings/python/sysrepo.py $(1)$(PYTHON3_PKG_DIR)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/bindings/python/_sysrepo.so $(1)$(PYTHON3_PKG_DIR)
endef

define Package/sysrepo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sysrepo-plugind $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/sysrepo.init $(1)/etc/init.d/sysrepo
endef

define Package/sysrepoctl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sysrepoctl $(1)/usr/bin/
endef

define Package/sysrepocfg/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sysrepocfg $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libsysrepo))
$(eval $(call BuildPackage,python3-sysrepo))
$(eval $(call BuildPackage,sysrepo))
$(eval $(call BuildPackage,sysrepoctl))
$(eval $(call BuildPackage,sysrepocfg))
