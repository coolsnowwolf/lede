include $(TOPDIR)/rules.mk

PKG_NAME:=ucert
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/ucert.git
PKG_SOURCE_DATE:=2018-08-08
PKG_SOURCE_VERSION:=1056e730f56adbc6bc77872e2bbd2bbe01512ec9
PKG_MIRROR_HASH:=59fc4ab5b5cb10c6e9e0899e3ed5de2f57926e980d31c13ddd1662f36cf40cef

CMAKE_INSTALL:=1
PKG_CHECK_FORMAT_SECURITY:=1

PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=COPYING

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_HOST_OPTIONS += \
	-DUCERT_FULL=1 \
	-DUCERT_HOST_BUILD=1 \
	-DCMAKE_SKIP_RPATH=FALSE \
	-DUSE_RPATH="${STAGING_DIR_HOST}/lib"

HOST_BUILD_DEPENDS:=libubox/host libjson-c/host usign/host
HOST_BUILD_PREFIX:=$(STAGING_DIR_HOST)
PKG_BUILD_DEPENDS:=ucert/host

define Package/ucert-full
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+usign +libubox +libjson-c +libblobmsg-json
  TITLE:=OpenWrt certificate generation and verification utility
  PROVIDES:=ucert
  VARIANT:=full
endef

define Package/ucert
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+usign +libubox
  TITLE:=OpenWrt certificate verification utility
  CONFLICTS:=ucert-full
  VARIANT:=tiny
endef

ifeq ($(BUILD_VARIANT),full)
  CMAKE_OPTIONS += -DUCERT_FULL=1
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(call Build/Prepare/Default)
endef

define Package/ucert/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ucert $(1)/usr/bin
endef

Package/ucert-full/install = $(Package/ucert/install)

$(eval $(call BuildPackage,ucert))
$(eval $(call BuildPackage,ucert-full))
$(eval $(call HostBuild))
