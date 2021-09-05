include $(INCLUDE_DIR)/cmake.mk
include ../../devel/ninja/ninja.mk

CMAKE_HOST_OPTIONS += -DCMAKE_GENERATOR="Ninja"
CMAKE_OPTIONS += -DCMAKE_GENERATOR="Ninja"

define Host/Compile/Default
	$(call Ninja,-C $(HOST_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR) $(1),)
endef

define Host/Install/Default
	$(call Ninja,-C $(HOST_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR) install,)
endef

define Host/Uninstall/Default
	-$(call Ninja,-C $(HOST_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR) uninstall,)
endef

define Build/Compile/Default
	$(call Ninja,-C $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR) $(1),)
endef

define Build/Install/Default
	$(call Ninja,-C $(PKG_BUILD_DIR)/$(CMAKE_BINARY_SUBDIR) install,DESTDIR="$(PKG_INSTALL_DIR)")
endef
