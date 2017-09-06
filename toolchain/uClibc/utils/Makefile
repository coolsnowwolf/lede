PATH_PREFIX=..

include ../common.mk

HOST_STAMP_BUILT:=$(HOST_BUILD_DIR)/.utils_built
HOST_STAMP_INSTALLED:=$(TOOLCHAIN_DIR)/stamp/.uclibc_utils_installed

define Host/Compile
	$(SED) 's,^CROSS=.*,CROSS=$(TARGET_CROSS),g' $(HOST_BUILD_DIR)/Rules.mak
	$(UCLIBC_MAKE) PREFIX= utils
endef

define Host/Install
	$(INSTALL_DIR) $(TOOLCHAIN_DIR)/bin
	$(INSTALL_BIN) \
		$(HOST_BUILD_DIR)/utils/ldd \
		$(TOOLCHAIN_DIR)/bin/
	$(INSTALL_DIR) $(TOOLCHAIN_DIR)/sbin
	$(INSTALL_BIN) \
		$(HOST_BUILD_DIR)/utils/ldconfig \
		$(TOOLCHAIN_DIR)/sbin/
endef

$(eval $(call HostBuild))
