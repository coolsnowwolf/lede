GCC_VARIANT:=minimal
GCC_PREPARE=$(if $(CONFIG_USE_MUSL),,1)

include ../common.mk

GCC_CONFIGURE += \
	--with-newlib \
	--without-headers \
	--enable-languages=c \
	--disable-libsanitizer \
	--disable-libssp \
	--disable-shared \
	--disable-threads

define Host/Compile
	+$(GCC_MAKE) $(HOST_JOBS) -C $(GCC_BUILD_DIR) all-gcc all-target-libgcc
endef

define Host/Install
	+$(GCC_MAKE) $(HOST_JOBS) -C $(GCC_BUILD_DIR) install-gcc install-target-libgcc
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(GCC_BUILD_DIR)
endef

$(eval $(call HostBuild))
