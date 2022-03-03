include $(TOPDIR)/rules.mk

PKG_NAME:=ubox
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/ubox.git
PKG_SOURCE_DATE:=2020-10-25
PKG_SOURCE_VERSION:=9ef886819dd48303d8ced4cdbc9afbf32682535c
PKG_MIRROR_HASH:=e2d93b798b91de98cb003f7b3be97d3d8a2413c8612b527d096c86ac5365cbdd
CMAKE_INSTALL:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=John Crispin <john@phrozen.org>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lrt)

define Package/ubox
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libubox +ubusd +ubus +libubus +libuci +USE_GLIBC:librt
  TITLE:=OpenWrt system helper toolbox
  ALTERNATIVES:=\
    100:/sbin/rmmod:/sbin/kmodloader \
    100:/sbin/insmod:/sbin/kmodloader \
    100:/sbin/lsmod:/sbin/kmodloader \
    100:/sbin/modinfo:/sbin/kmodloader \
    100:/sbin/modprobe:/sbin/kmodloader
endef

define Package/getrandom
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=OpenWrt getrandom system helper
endef

define Package/logd
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libubox +libubus +libblobmsg-json +USE_GLIBC:librt
  TITLE:=OpenWrt system log implementation
  USERID:=logd=514:logd=514
endef

define Package/getrandom/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/getrandom $(1)/usr/bin/
endef

define Package/ubox/install
	$(INSTALL_DIR) $(1)/sbin $(1)/usr/sbin $(1)/lib $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{kmodloader,validate_data} $(1)/sbin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libvalidate.so $(1)/lib
endef

define Package/logd/install
	$(INSTALL_DIR) $(1)/sbin $(1)/etc/init.d/

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{logd,logread} $(1)/sbin/
	$(INSTALL_BIN) ./files/log.init $(1)/etc/init.d/log
endef

$(eval $(call BuildPackage,ubox))
$(eval $(call BuildPackage,getrandom))
$(eval $(call BuildPackage,logd))
