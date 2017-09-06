define Build/Compile
	$(call Build/Compile/Default)
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/adam2patcher $(PLATFORM_DIR)/src/adam2patcher.c
endef

define Package/base-files/install-target
	mkdir -p $(1)/sbin
	$(CP) $(PKG_BUILD_DIR)/adam2patcher $(1)/sbin
endef


