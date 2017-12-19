PATH_PREFIX=.

include ./common.mk

HOST_STAMP_BUILT:=$(HOST_BUILD_DIR)/.built
HOST_STAMP_INSTALLED:=$(TOOLCHAIN_DIR)/stamp/.uclibc_installed

HOST_BUILD_PARALLEL:=1

define Host/SetToolchainInfo
	$(SED) 's,^\(LIBC_TYPE\)=.*,\1=$(PKG_NAME),' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_URL\)=.*,\1=http://www.uclibc.org/,' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_VERSION\)=.*,\1=$(PKG_VERSION),' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_SO_VERSION\)=.*,\1=$(LIBC_SO_VERSION),' $(TOOLCHAIN_DIR)/info.mk
endef

define Host/Compile
	$(SED) 's,^CROSS=.*,CROSS=$(TARGET_CROSS),g' $(HOST_BUILD_DIR)/Rules.mak
	$(UCLIBC_MAKE) PREFIX= all
endef

define Host/Install
	$(call Host/SetToolchainInfo)
	$(UCLIBC_MAKE) PREFIX="$(TOOLCHAIN_DIR)/" install_runtime install_dev
	$(CP) $(HOST_BUILD_DIR)/libc/libc_so.a $(TOOLCHAIN_DIR)/lib/
	( cd $(TOOLCHAIN_DIR) ; \
		for d in lib usr/lib ; do \
		  for f in libc.so libgcc_s.so ; do \
		    if [ -f $$$$d/$$$$f -a ! -L $$$$d/$$$$f ] ; then \
		      $(SED) 's,/usr/lib/,,g;s,/lib/,,g' $$$$d/$$$$f ; \
		    fi \
		  done \
		done \
	)
endef

$(eval $(call HostBuild))
