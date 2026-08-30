include $(TOPDIR)/rules.mk

PKG_NAME:=usign
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/usign.git
PKG_SOURCE_DATE:=2025-10-03
PKG_SOURCE_VERSION:=c4c72b1b07945ee192361dc751291a7c98d6adcd
PKG_MIRROR_HASH:=889e58b295505bdde749ec260414fd24411ce2437e5049a7d5cf4e410e8cbe9d
CMAKE_INSTALL:=1
PKG_CHECK_FORMAT_SECURITY:=1
PKG_BUILD_FLAGS:=no-mips16 lto

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

HOST_BUILD_PREFIX:=$(STAGING_DIR_HOST)

define Package/usign
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libubox
  TITLE:=OpenWrt signature verification utility
endef

CMAKE_OPTIONS += \
	-DUSE_LIBUBOX=on

define Package/usign/install
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/usign $(1)/usr/bin
	ln -s usign $(1)/usr/bin/signify
endef

$(eval $(call BuildPackage,usign))
$(eval $(call HostBuild))
