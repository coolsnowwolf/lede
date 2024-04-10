include $(TOPDIR)/rules.mk

PKG_NAME:= quectel-CM-5G
PKG_VERSION:=1.6.5
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/quectel-CM-5G
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=quectel-CM-5G app
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		CC="$(TARGET_CC)"
endef

define Package/quectel-CM-5G/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/lib/netifd/proto
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-CM $(1)/usr/bin
	$(INSTALL_BIN) ./files/rmnet_init.sh $(1)/lib/netifd
	$(INSTALL_BIN) ./files/rmnet.script $(1)/lib/netifd
	$(INSTALL_BIN) ./files/rmnet.sh $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/rmnet6.sh $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/rmnet6.script $(1)/lib/netifd
endef

$(eval $(call BuildPackage,quectel-CM-5G))
