include $(TOPDIR)/rules.mk

PKG_NAME:=ucert
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/ucert.git
PKG_SOURCE_DATE:=2025-10-03
PKG_SOURCE_VERSION:=57270b247c91f003db6e3ba1a71d6d1fa5710fef
PKG_MIRROR_HASH:=a926fefa957f42c40caa9f8592f4c63a0a56af9094a9995eb0dfc5d82c9591cf

CMAKE_INSTALL:=1
PKG_CHECK_FORMAT_SECURITY:=1

PKG_LICENSE:=GPL-3.0+
PKG_LICENSE_FILES:=COPYING

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

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

define Package/ucert/Default
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+usign +libubox
endef

define Package/ucert
$(call Package/ucert/Default)
  TITLE:=OpenWrt certificate verification utility
  VARIANT:=tiny
  DEFAULT_VARIANT:=1
endef

define Package/ucert-full
$(call Package/ucert/Default)
  TITLE:=OpenWrt certificate generation and verification utility
  DEPENDS+=+libjson-c +libblobmsg-json
  VARIANT:=full
  PROVIDES:=ucert
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
