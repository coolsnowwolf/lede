PATH_PREFIX:=..
VARIANT:=headers

include ../common.mk

define Host/Compile

endef

define Host/Install
	$(call Host/SetToolchainInfo)
	mkdir -p $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/{include,lib}
	$(MAKE) -C $(CUR_BUILD_DIR) \
		BUILD_CFLAGS="$(HOST_CFLAGS)" \
		install_root="$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev" \
		install-bootstrap-headers=yes \
		install-headers
	$(CP) $(BUILD_DIR_TOOLCHAIN)/linux-dev/* $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/
	$(MAKE) -C $(CUR_BUILD_DIR) \
		csu/subdir_lib
	( cd $(CUR_BUILD_DIR); \
		$(CP) csu/crt1.o csu/crti.o csu/crtn.o $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/lib/ \
	)
	$(TARGET_CC) -nostdlib -nostartfiles -shared -x c /dev/null \
		-o $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/lib/libc.so
endef

$(eval $(call HostBuild))
