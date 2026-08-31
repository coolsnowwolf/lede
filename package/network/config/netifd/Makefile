include $(TOPDIR)/rules.mk

PKG_NAME:=netifd
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/netifd.git
PKG_SOURCE_DATE:=2026-07-08
PKG_SOURCE_VERSION:=6088f7b3b9d756131619c7fb3b520bf0cc0ec0d2
PKG_MIRROR_HASH:=6621f8462ca6add377090b76b21e59d54ab28b08ff66017dc9d3f19203587660
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_FLAGS:=lto

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/netifd
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libuci +libnl-tiny +libubus +ubus +ubusd +jshn +libubox +libudebug \
	+ucode +ucode-mod-fs +ucode-mod-ubus +ucode-mod-uloop +ucode-mod-uci
  TITLE:=OpenWrt Network Interface Configuration Daemon
endef

define Package/netifd/conffiles
/etc/udhcpc.user
/etc/udhcpc.user.d/
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(STAGING_DIR)/usr/include

CMAKE_OPTIONS += \
	-DLIBNL_LIBS=-lnl-tiny \
	-DDEBUG=1

define Package/netifd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netifd $(1)/sbin/
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/etc/udhcpc.user.d/
	$(CP) \
		$(PKG_BUILD_DIR)/scripts/utils.sh \
		$(PKG_BUILD_DIR)/scripts/netifd-proto.sh \
		$(1)/lib/netifd/
endef

$(eval $(call BuildPackage,netifd))
