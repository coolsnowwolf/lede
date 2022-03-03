PATH_PREFIX=.

include ./common.mk

HOST_STAMP_BUILT:=$(HOST_BUILD_DIR)/.built
HOST_STAMP_INSTALLED:=$(TOOLCHAIN_DIR)/stamp/.musl_installed

HOST_BUILD_PARALLEL:=1

MUSL_MAKEOPTS = -C $(HOST_BUILD_DIR) \
	DESTDIR="$(TOOLCHAIN_DIR)/" \
	LIBCC="$(subst libgcc.a,libgcc_initial.a,$(shell $(TARGET_CC) -print-libgcc-file-name))"

define Host/SetToolchainInfo
	$(SED) 's,^\(LIBC_TYPE\)=.*,\1=$(PKG_NAME),' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_URL\)=.*,\1=http://www.musl-libc.org/,' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_VERSION\)=.*,\1=$(PKG_VERSION),' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_SO_VERSION\)=.*,\1=$(LIBC_SO_VERSION),' $(TOOLCHAIN_DIR)/info.mk
endef

define Host/Compile
	+$(MAKE) $(HOST_JOBS) $(MUSL_MAKEOPTS) all
endef

define Host/Install
	$(call Host/SetToolchainInfo)
	+$(MAKE) $(HOST_JOBS) $(MUSL_MAKEOPTS) DESTDIR="$(TOOLCHAIN_DIR)/" install
	$(CP) ./include $(TOOLCHAIN_DIR)/
endef

$(eval $(call HostBuild))
