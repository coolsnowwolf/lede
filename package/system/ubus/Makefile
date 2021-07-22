include $(TOPDIR)/rules.mk

PKG_NAME:=ubus
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/ubus.git
PKG_SOURCE_DATE:=2021-06-03
PKG_SOURCE_VERSION:=a8cf678230ed163ff7a07eb1e2c872f9d655460a
PKG_MIRROR_HASH:=aee34cd1c1aa0f1a459dda0b2c6cbdb6b66e67147ebd1bcbb2a16a2ef923d008
PKG_ABI_VERSION:=$(call abi_version_str,$(PKG_SOURCE_DATE))
CMAKE_INSTALL:=1

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_FLAGS := nonshared
PKG_ASLR_PIE_REGULAR:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/ubus
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libubus +libblobmsg-json +ubusd
  TITLE:=OpenWrt RPC client utility
endef

define Package/ubusd
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=OpenWrt RPC daemon
  DEPENDS:=+libubox +libblobmsg-json
  USERID:=ubus=81:ubus=81
endef

define Package/libubus
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libubox
  ABI_VERSION:=$(PKG_ABI_VERSION)
  TITLE:=OpenWrt RPC client library
endef

define Package/libubus-lua
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libubus +liblua
  TITLE:=Lua binding for the OpenWrt RPC client
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -flto
TARGET_LDFLAGS += -flto

CMAKE_OPTIONS += \
	-DLUAPATH=/usr/lib/lua \
	-DABIVERSION="$(PKG_ABI_VERSION)"

define Package/ubus/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ubus $(1)/bin/
endef

define Package/ubusd/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/ubusd $(1)/sbin/
endef

define Package/libubus/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libubus.so.* $(1)/lib/
endef

define Package/libubus-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/lua/ubus.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,libubus))
$(eval $(call BuildPackage,libubus-lua))
$(eval $(call BuildPackage,ubus))
$(eval $(call BuildPackage,ubusd))
