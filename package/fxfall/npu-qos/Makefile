include $(TOPDIR)/rules.mk

PKG_NAME:=npu-qos
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Package/npu-qos
  SECTION:=net
  CATEGORY:=Network
  TITLE:=SR1010 managed NPU shaper control
  DEPENDS:=@TARGET_zte_zx279133_DEVICE_zte_zxslc-sr1010 +kmod-zte-npu
endef

define Package/npu-qos/description
 Small production control client for the SR1010 managed NPU shaper API.
 Does not include the diagnostic npu-debug utility.
endef

define Build/Compile
endef

define Package/npu-qos/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/npu-qos $(1)/usr/sbin/npu-qos
endef

$(eval $(call BuildPackage,npu-qos))
