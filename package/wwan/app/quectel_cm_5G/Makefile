include $(TOPDIR)/rules.mk

PKG_NAME:= quectel-CM-5G
PKG_RELEASE:=2
PKG_VERSION:=1.0

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/quectel-CM-5G
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=quectel-CM-5G app building test
endef

define Package/quectel-CM-5G/description
	quectel-CM-5G app building test
endef

define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		CC="$(TARGET_CC)"
endef

define Package/quectel-CM-5G/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/lib/netifd/proto $(1)/lib/netifd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-CM $(1)/usr/bin/quectel-CM
	$(INSTALL_BIN) ./files/rmnet.sh $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/rmnet6.sh $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/rmnet6.script $(1)/lib/netifd
	$(INSTALL_BIN) ./files/rmnet.script $(1)/lib/netifd
	$(INSTALL_BIN) ./files/rmnet_init.sh $(1)/usr/bin
endef

$(eval $(call BuildPackage,quectel-CM-5G))
