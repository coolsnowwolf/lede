GCC_VARIANT:=initial
GCC_PREPARE=$(CONFIG_USE_MUSL)

include ../common.mk

GCC_CONFIGURE += \
	--with-newlib \
	--with-sysroot=$(TOOLCHAIN_DIR) \
	--enable-languages=c \
	--disable-shared \
	--disable-threads \

define Host/Compile
	+$(GCC_MAKE) $(HOST_JOBS) -C $(GCC_BUILD_DIR) \
		all-build-libiberty \
		all-gcc \
		all-target-libgcc
endef

define Host/Install
	+$(GCC_MAKE) $(HOST_JOBS) -C $(GCC_BUILD_DIR) \
		install-gcc \
		install-target-libgcc

	$(call FixupLibdir,$(TOOLCHAIN_DIR))
endef

$(eval $(call HostBuild))
