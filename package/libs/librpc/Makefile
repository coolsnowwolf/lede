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

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk
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

$(eval $(call BuildPackage,librpc))
