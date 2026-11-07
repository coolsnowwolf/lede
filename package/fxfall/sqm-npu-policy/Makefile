include $(TOPDIR)/rules.mk

PKG_NAME:=sqm-npu-policy
PKG_RELEASE:=10

include $(INCLUDE_DIR)/package.mk

define Package/sqm-npu-policy
  SECTION:=net
  CATEGORY:=Network
  TITLE:=SR1010 NPU SQM policy backend
  DEPENDS:=@TARGET_zte_zx279133_DEVICE_zte_zxslc-sr1010 +npu-qos
endef

define Package/sqm-npu-policy/description
 Configure SR1010 NPU WAN shaping and per-direction flow queue selection.
 This does not install software CAKE or manage LAN-to-LAN traffic.
endef

define Package/sqm-npu-policy/conffiles
/etc/config/sqm-npu
endef

define Package/sqm-npu-policy/install
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_CONF) ./files/sqm-npu.config $(1)/etc/config/sqm-npu
	$(INSTALL_BIN) ./files/sqm-npu.init $(1)/etc/init.d/sqm-npu
endef

define Build/Compile
endef

$(eval $(call BuildPackage,sqm-npu-policy))
