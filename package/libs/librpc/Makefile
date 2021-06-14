include $(TOPDIR)/rules.mk

PKG_NAME:=librpc
PKG_RELEASE=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/librpc-uclibc.git
PKG_SOURCE_DATE:=2015-11-04
PKG_SOURCE_VERSION:=a921e3ded051746f9f7cd5e5a312fb6771716aac
PKG_MIRROR_HASH:=22c8dc55e1c4e8e31635a37708a3ce622a6ca33ebd918a4321b0be6ffce89b21
CMAKE_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_DEPENDS:=librpc/host

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/librpc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=uClibc RPC library
endef

define Package/librpc/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/librpc.so $(1)/lib/
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/include/rpc
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/rpc/types.h $(STAGING_DIR_HOSTPKG)/include/rpc
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/rpc/compat.h $(STAGING_DIR_HOSTPKG)/include/rpc
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,librpc))
