PATH_PREFIX:=..

include ../common.mk

HOST_STAMP_BUILT:=$(HOST_BUILD_DIR)/.headers_built
HOST_STAMP_INSTALLED:=$(TOOLCHAIN_DIR)/stamp/.uclibc_headers_installed

define Host/Compile

endef

define Host/Install
	PATH='$(TARGET_PATH)' $(MAKE) -C $(HOST_BUILD_DIR) \
		PREFIX="$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/" \
		DEVEL_PREFIX=/ \
		RUNTIME_PREFIX="$(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/" \
		HOSTCC="$(HOSTCC)" \
		CC="$(TARGET_CC)" \
		CPU_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(CONFIG_ARCH)" \
		pregen \
		install_headers
	$(CP) $(BUILD_DIR_TOOLCHAIN)/linux-dev/* $(BUILD_DIR_TOOLCHAIN)/$(LIBC)-dev/
endef

$(eval $(call HostBuild))

