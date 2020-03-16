
include $(TOPDIR)/rules.mk

PKG_NAME:=appfilter
PKG_VERSION:=3.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
#include $(INCLUDE_DIR)/kernel.mk

define Package/appfilter
  SECTION:=Derry Apps
  CATEGORY:=Derry Apps
  TITLE:=App filter userspace module
endef

define Build/Prepare
        mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default

endef

define Package/appfilter/description
  open appfilter app
endef


define Package/appfilter/install
	echo "install"
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/appfilter
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) ./files/feature.cfg $(1)/etc/appfilter/ 
	$(INSTALL_BIN) ./files/appfilter.init $(1)/etc/init.d/appfilter
	$(INSTALL_BIN) ./files/appfilter.sh $(1)/usr/bin
	$(INSTALL_BIN) ./files/gen_class.sh $(1)/usr/bin
	$(INSTALL_BIN) ./files/appfilter.config $(1)/etc/config/appfilter
endef


$(eval $(call BuildPackage,appfilter))

