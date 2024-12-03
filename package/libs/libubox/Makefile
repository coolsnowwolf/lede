include $(TOPDIR)/rules.mk

PKG_NAME:=libubox
PKG_RELEASE=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/libubox.git
PKG_MIRROR_HASH:=242e33eca235124c7e005d25fbc8f8bf08a324335343e60278d4535c91157ba4
PKG_SOURCE_DATE:=2024-03-29
PKG_SOURCE_VERSION:=eb9bcb64185ac155c02cc1a604692c4b00368324
PKG_ABI_VERSION:=$(call abi_version_str,$(PKG_SOURCE_DATE))
CMAKE_INSTALL:=1

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_BUILD_DEPENDS:=lua
HOST_BUILD_DEPENDS:=libjson-c/host
HOST_BUILD_PREFIX:=$(STAGING_DIR_HOST)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libubox
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Basic utility library
  ABI_VERSION:=$(PKG_ABI_VERSION)
  DEPENDS:=
endef

define Package/libblobmsg-json
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=blobmsg <-> json conversion library
  ABI_VERSION:=$(PKG_ABI_VERSION)
  DEPENDS:=+libjson-c +libubox
endef

define Package/jshn
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libjson-c +libubox +libblobmsg-json
  TITLE:=JSON SHell Notation
endef

define Package/jshn/description
  Library for parsing and generating JSON from shell scripts
endef

define Package/libjson-script
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libubox
  ABI_VERSION:=$(PKG_ABI_VERSION)
  TITLE:=Minimalistic JSON based scripting engine
endef

define Package/libubox-lua
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libubox +liblua
  TITLE:=Lua binding for the OpenWrt Basic utility library
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
CMAKE_OPTIONS += \
	-DLUAPATH=/usr/lib/lua \
	-DABIVERSION="$(PKG_ABI_VERSION)"

define Package/libubox/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libubox.so.* $(1)/lib/
endef

define Package/libblobmsg-json/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libblobmsg_json.so.* $(1)/lib/
endef

define Package/jshn/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/libubox
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/jshn $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/libubox/jshn.sh $(1)/usr/share/libubox
endef

define Package/libjson-script/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libjson_script.so.* $(1)/lib/
endef

define Package/libubox-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/lua/uloop.so $(1)/usr/lib/lua/
endef


CMAKE_HOST_OPTIONS += \
	-DBUILD_LUA=OFF \
	-DBUILD_EXAMPLES=OFF \
	-DCMAKE_SKIP_RPATH=FALSE \
	-DCMAKE_MACOSX_RPATH=1 \
	-DCMAKE_INSTALL_RPATH="${STAGING_DIR_HOST}/lib" \

ifeq ($(HOST_OS),Darwin)
  define Host/Install
	$(Host/Install/Default)
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/lib
	cd "$(STAGING_DIR_HOSTPKG)/lib" && ln -sf ../../host/lib/libubox.* .
  endef
endif

$(eval $(call BuildPackage,libubox))
$(eval $(call BuildPackage,libblobmsg-json))
$(eval $(call BuildPackage,jshn))
$(eval $(call BuildPackage,libjson-script))
$(eval $(call BuildPackage,libubox-lua))
$(eval $(call HostBuild))
