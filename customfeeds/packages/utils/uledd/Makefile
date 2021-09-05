include $(TOPDIR)/rules.mk

PKG_NAME:=uledd
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/blogic/uledd.git
PKG_SOURCE_DATE:=2020-09-15
PKG_SOURCE_VERSION:=21d14d950bfe702b8c1f77de20f7529266dbd0f7
PKG_MIRROR_HASH:=9ccb019123d1287b7f8750d1733e6ab47d2abfc3f37ce113a5f682ed51eee309

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=LGPL-2.1-only
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/uledd
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libubox +libubus +libblobmsg-json +libjson-c
  TITLE:=PWM/RGB LED ubus frontend
endef

define Package/uledd/install
	$(INSTALL_DIR) $(1)/sbin $(1)/etc/init.d

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/uledd $(1)/sbin/
	$(INSTALL_BIN) ./files/uledd.init $(1)/etc/init.d/uledd
endef

$(eval $(call BuildPackage,uledd))
