include $(TOPDIR)/rules.mk

PKG_NAME:=nlbwmon
PKG_RELEASE:=5

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/jow-/nlbwmon.git
PKG_SOURCE_DATE:=2020-04-11.1
PKG_SOURCE_VERSION:=34a188769750927d839ad27fbfc90c973fe7bb06
PKG_MIRROR_HASH:=75cd4e2eccb91256a91a3d48440cb347a696db64db2e71c050d284d20f98a77c

CMAKE_INSTALL:=1

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DLIBNL_LIBRARY_TINY=ON
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

define Package/nlbwmon
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libubox +libnl-tiny +zlib +kmod-nf-conntrack-netlink
  TITLE:=OpenWrt Traffic Usage Monitor
endef

define Package/nlbwmon/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nlbwmon $(1)/usr/sbin/nlbwmon
	$(LN) nlbwmon $(1)/usr/sbin/nlbw
	$(INSTALL_DIR) $(1)/usr/share/nlbwmon
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/protocols.txt $(1)/usr/share/nlbwmon/protocols
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/nlbwmon.init $(1)/etc/init.d/nlbwmon
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/nlbwmon.config $(1)/etc/config/nlbwmon
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/nlbwmon.hotplug $(1)/etc/hotplug.d/iface/30-nlbwmon
endef

define Package/nlbwmon/conffiles
/etc/config/nlbwmon
/usr/share/nlbwmon/protocols
endef


$(eval $(call BuildPackage,nlbwmon))
