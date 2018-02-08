#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
PKG_NAME:=toolchain
PKG_RELEASE:=1

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-3.0-with-GCC-exception

PKG_FLAGS:=hold essential nonshared

include $(INCLUDE_DIR)/package.mk

ifneq ($(DUMP),1)
  LIBGCC_VERSION:=$(GCC_VERSION)
else
  LIBC_VERSION:=<LIBC_VERSION>
  LIBGCC_VERSION:=<LIBGCC_VERSION>
endif

define Package/gcc/Default
  SECTION:=libs
  CATEGORY:=Base system
  URL:=http://gcc.gnu.org/
  VERSION:=$(LIBGCC_VERSION)-$(PKG_RELEASE)
endef

define Package/libgcc
$(call Package/gcc/Default)
  TITLE:=GCC support library
endef

define Package/libgcc/config
	menu "Configuration"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgcc

	config LIBGCC_ROOT_DIR
		string
		prompt "libgcc shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgcc
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBGCC_FILE_SPEC
		string
		prompt "libgcc shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgcc
		default "./lib/libgcc_s.so.*"

	endmenu
endef

define Package/libatomic
$(call Package/gcc/Default)
  DEPENDS:=+libgcc
  TITLE:=Atomic support library
endef

define Package/libatomic/config
	menu "Configuration"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libatomic

	config LIBATOMIC_ROOT_DIR
		string
		prompt "libatomic shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libatomic
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBATOMIC_FILE_SPEC
		string
		prompt "libatomic shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libatomic
		default "./lib/libatomic.so.*"

	endmenu
endef

define Package/libssp
$(call Package/gcc/Default)
  DEPENDS+=@GCC_LIBSSP
  TITLE:=GCC support library
endef

define Package/libssp/config
	menu "Configuration"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libssp

	config LIBSSP_ROOT_DIR
		string
		prompt "libssp shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libssp
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBSSP_FILE_SPEC
		string
		prompt "libssp shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libssp
		default "./lib/libssp.so.*"

	endmenu
endef


define Package/libstdcpp
$(call Package/gcc/Default)
  NAME:=libstdc++
  TITLE:=GNU Standard C++ Library v3
endef

define Package/libstdcpp/config
	menu "Configuration"
	depends on EXTERNAL_TOOLCHAIN && PACKAGE_libstdcpp

	config LIBSTDCPP_ROOT_DIR
		string
		prompt "libstdcpp shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libstdcpp
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBSTDCPP_FILE_SPEC
		string
		prompt "libstdc++ shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libstdcpp
		default "./lib/libstdc++.so.*"

	endmenu
endef


define Package/libc/Default
  SECTION:=libs
  CATEGORY:=Base system
  VERSION:=$(LIBC_VERSION)-$(PKG_RELEASE)
  DEPENDS:=+libgcc
  URL:=$(LIBC_URL)
endef


define Package/libc
$(call Package/libc/Default)
  TITLE:=C library
endef

define Package/libc/config
	menu "Configuration"
	depends on EXTERNAL_TOOLCHAIN && PACKAGE_libc

	config LIBC_ROOT_DIR
		string
		prompt "libc shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libc
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBC_FILE_SPEC
		string
		prompt "libc shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libc
		default "./lib/ld{*.so*,-linux*.so.*} ./lib/lib{anl,c,cidn,crypt,dl,m,nsl,nss_dns,nss_files,resolv,util}{-*.so,.so.*,.so}"

	endmenu
endef


define Package/libpthread
$(call Package/libc/Default)
  TITLE:=POSIX thread library
endef

define Package/libpthread/config
	menu "Configuration"
	depends on EXTERNAL_TOOLCHAIN && PACKAGE_libpthread

	config LIBPTHREAD_ROOT_DIR
		string
		prompt "libpthread shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libpthread
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBPTHREAD_FILE_SPEC
		string
		prompt "libpthread shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libpthread
		default "./lib/libpthread{-*.so,.so.*}"

	endmenu
endef


define Package/libthread-db
$(call Package/libc/Default)
  DEPENDS:=@!USE_MUSL
  TITLE:=POSIX thread library debugging support
endef

define Package/libthread-db/config
	menu "Configuration"
	depends on EXTERNAL_TOOLCHAIN && PACKAGE_libthread-db

	config LIBTHREAD_DB_ROOT_DIR
		string
		prompt "POSIX thread debugging shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libthread-db
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBTHREAD_DB_FILE_SPEC
		string
		prompt "POSIX thread debugging shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libthread-db
		default "./lib/libthread_db{-*.so,.so.*}"

	endmenu
endef

define Package/librt
$(call Package/libc/Default)
  TITLE:=POSIX.1b RealTime extension library
  DEPENDS:=+libpthread
endef

define Package/librt/config
	menu "Configuration"
	depends on EXTERNAL_TOOLCHAIN && PACKAGE_librt

	config LIBRT_ROOT_DIR
		string
		prompt "librt shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_librt
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBRT_FILE_SPEC
		string
		prompt "librt shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_librt
		default "./lib/librt{-*.so,.so.*}"

	endmenu
endef


define Package/libgfortran
$(call Package/gcc/Default)
  TITLE:=GFortran support library
  DEPENDS+=@INSTALL_GFORTRAN
endef

define Package/libgfortran/config
	menu "Configuration"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgfortran

	config LIBGFORTRAN_ROOT_DIR
		string
		prompt "libgfortran shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgfortran
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBGFORTRAN_FILE_SPEC
		string
		prompt "libgfortran shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgfortran
		default "./usr/lib/libgfortran.so.*"

	endmenu
endef

define Package/libgomp
$(call Package/gcc/Default)
  TITLE:=OpenMP support library
endef

define Package/libgomp/config
	menu "Configuration"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgomp

	config LIBGOMP_ROOT_DIR
		string
		prompt "libgomp shared library base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgomp
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LIBGOMP_FILE_SPEC
		string
		prompt "libgomp shared library files (use wildcards)"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_libgomp
		default "./lib/libgomp.so*"

	endmenu
endef


define Package/ldd
$(call Package/libc/Default)
  DEPENDS:=@!USE_MUSL
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=LDD trace utility
endef

define Package/ldd/config
	menu "Configuration"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_ldd

	config LDD_ROOT_DIR
		string
		prompt "ldd trace utility base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_ldd
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LDD_FILE_SPEC
		string
		prompt "ldd trace utility file"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_ldd
		default "./usr/bin/ldd"

	endmenu
endef


define Package/ldconfig
$(call Package/libc/Default)
  DEPENDS:=@!USE_MUSL
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Shared library path configuration
endef

define Package/ldconfig/config
	menu "Configuration"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_ldconfig

	config LDCONFIG_ROOT_DIR
		string
		prompt "ldconfig base directory"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_ldconfig
		default TOOLCHAIN_ROOT  if !NATIVE_TOOLCHAIN
		default "/"  if NATIVE_TOOLCHAIN

	config LDCONFIG_FILE_SPEC
		string
		prompt "ldconfig file"
		depends on EXTERNAL_TOOLCHAIN && PACKAGE_ldconfig
		default "./sbin/ldconfig"

	endmenu
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

LIBGCC_A=$(lastword $(wildcard $(TOOLCHAIN_DIR)/lib/gcc/*/*/libgcc_pic.a))
LIBGCC_MAP=$(lastword $(wildcard $(TOOLCHAIN_DIR)/lib/gcc/*/*/libgcc.map))
LIBGCC_SO=$(lastword $(wildcard $(TOOLCHAIN_DIR)/lib/libgcc_s.so.*))

define Build/Compile/uClibc
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/libuClibc-*.so \
		$(PKG_BUILD_DIR)/
endef
ifneq ($(LIBGCC_SO),)
    define Build/Compile/libgcc
	$(CP) $(LIBGCC_SO) $(PKG_BUILD_DIR)/
    endef
endif

define Build/Compile/Default
	$(call Build/Compile/libgcc)
	$(call Build/Compile/$(LIBC))
endef
Build/Compile = $(Build/Compile/Default)

ifeq ($(CONFIG_EXTERNAL_TOOLCHAIN),)

  define Package/libgcc/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(TOOLCHAIN_DIR)/lib/libgcc_s.so.* $(1)/lib/
  endef

  define Package/libatomic/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(TOOLCHAIN_DIR)/lib/libatomic.so.* $(1)/lib/
  endef

  define Package/libgfortran/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(TOOLCHAIN_DIR)/lib/libgfortran.so.* $(1)/usr/lib/
  endef

  define Package/libssp/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(TOOLCHAIN_DIR)/lib/libssp.so.* $(1)/lib/
  endef

  define Package/libstdcpp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(TOOLCHAIN_DIR)/lib/libstdc++.so.* $(1)/usr/lib/
  endef

  define Package/glibc/install
	$(CP) ./glibc-files/* $(1)/
	rm -f $(1)/etc/localtime
	$(LN) /tmp/localtime $(1)/etc/localtime
	$(INSTALL_DIR) $(1)/lib
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/ld*.so.* \
		$(TOOLCHAIN_DIR)/lib/ld-$(LIBC_SO_VERSION).so \
		$(1)/lib/
	for file in libanl libc libcidn libcrypt libdl libm libnsl libnss_dns libnss_files libresolv libutil; do \
		for file in $(TOOLCHAIN_DIR)/lib/$$$$file.so.* $(TOOLCHAIN_DIR)/lib/$$$$file-$(LIBC_SO_VERSION).so; do \
			if [ -e "$$$$file" ]; then \
				$(CP) $$$$file $(1)/lib/; \
			fi; \
		done; \
	done
  endef

  define Package/uClibc/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/ld*-uClibc.so.* \
		$(TOOLCHAIN_DIR)/lib/ld*-uClibc-$(LIBC_SO_VERSION).so \
		$(1)/lib/
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/libc.so.* \
		$(TOOLCHAIN_DIR)/lib/libuClibc-$(LIBC_SO_VERSION).so \
		$(1)/lib/
	$(CP) \
		$(PKG_BUILD_DIR)/libuClibc-* \
		$(1)/lib/
  endef

  LD_MUSL_NAME = $(notdir $(firstword $(wildcard $(TOOLCHAIN_DIR)/lib/libc.so*)))

  define Package/musl/install
	$(INSTALL_DIR) $(1)/lib $(1)/usr/bin
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/ld-musl-*.so* \
		$(1)/lib/
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/libc.so* \
		$(1)/lib/
	$(LN) ../../lib/$(LD_MUSL_NAME) $(1)/usr/bin/ldd
  endef

  define Package/libc/install
    $(call Package/$(LIBC)/install,$1)
  endef

  define Package/libc/install_lib
	$(CP) $(filter-out %/libdl_pic.a %/libpthread_pic.a %/libresolv_pic.a,$(wildcard $(TOOLCHAIN_DIR)/lib/lib*.a)) $(1)/lib/
	$(if $(wildcard $(TOOLCHAIN_DIR)/lib/libc_so.a),$(CP) $(TOOLCHAIN_DIR)/lib/libc_so.a $(1)/lib/libc_pic.a)
	$(if $(LIBGCC_MAP), \
		$(CP) $(LIBGCC_A) $(1)/lib/libgcc_s_pic.a; \
		$(CP) $(LIBGCC_MAP) $(1)/lib/libgcc_s_pic.map \
	)
  endef

  define Package/libpthread/install
	$(INSTALL_DIR) $(1)/lib
  ifneq ($(CONFIG_USE_MUSL)$(CONFIG_USE_UCLIBC),y)
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/libpthread.so.* \
		$(TOOLCHAIN_DIR)/lib/libpthread-$(LIBC_SO_VERSION).so \
		$(1)/lib/
  endif
  endef

  define Package/libthread-db/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/libthread_db.so.* $(1)/lib
  ifeq ($(CONFIG_USE_UCLIBC),y)
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/libthread_db-$(LIBC_SO_VERSION).so \
		$(1)/lib/
  endif
  endef

  define Package/libpthread/install_lib
	$(if $(wildcard $(TOOLCHAIN_DIR)/lib/libpthread_so.a),$(CP) $(TOOLCHAIN_DIR)/lib/libpthread_so.a $(1)/lib/libpthread_pic.a)
  endef

  define Package/librt/install
	$(INSTALL_DIR) $(1)/lib
  ifneq ($(CONFIG_USE_MUSL)$(CONFIG_USE_UCLIBC),y)
	$(CP) \
		$(TOOLCHAIN_DIR)/lib/librt.so.* \
		$(TOOLCHAIN_DIR)/lib/librt-$(LIBC_SO_VERSION).so \
		$(1)/lib/
  endif
  endef

  define Package/ldd/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(TOOLCHAIN_DIR)/bin/ldd $(1)/usr/bin/
  endef

  define Package/ldconfig/install
	$(INSTALL_DIR) $(1)/sbin/
	$(CP) $(TOOLCHAIN_DIR)/sbin/ldconfig $(1)/sbin/
  endef

else

  define Package/libgcc/install
	for file in $(call qstrip,$(CONFIG_LIBGCC_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBGCC_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/libgfortran/install
	for file in $(call qstrip,$(CONFIG_LIBGFORTRAN_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBGFORTRAN_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done
  endef

  define Package/libssp/install
	for file in $(call qstrip,$(CONFIG_LIBSSP_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBSSP_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/libstdcpp/install
	for file in $(call qstrip,$(CONFIG_LIBSTDCPP_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBSTDCPP_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/libc/install
	for file in $(call qstrip,$(CONFIG_LIBC_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBC_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/libpthread/install
	for file in $(call qstrip,$(CONFIG_LIBPTHREAD_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBPTHREAD_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/libthread-db/install
	for file in $(call qstrip,$(CONFIG_LIBTHREAD_DB_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBTHREAD_DB_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/librt/install
	for file in $(call qstrip,$(CONFIG_LIBRT_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBRT_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/libatomic/install
	for file in $(call qstrip,$(CONFIG_LIBATOMIC_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBATOMIC_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/libgomp/install
	for file in $(call qstrip,$(CONFIG_LIBGOMP_FILE_SPEC)); do \
		$(INSTALL_DIR) $(1)/lib ; \
		$(CP) $(call qstrip,$(CONFIG_LIBGOMP_ROOT_DIR))/$$$$file $(1)/lib/ ; \
	done ; \
	exit 0
  endef

  define Package/ldd/install
	for file in $(call qstrip,$(CONFIG_LDD_FILE_SPEC)); do \
		dir=`dirname $$$$file` ; \
		$(INSTALL_DIR) $(1)/$$$$dir ; \
		$(CP) $(call qstrip,$(CONFIG_LDD_ROOT_DIR))/$$$$file $(1)/$$$$dir/ ; \
	done ; \
	exit 0
  endef

  define Package/ldconfig/install
	for file in $(call qstrip,$(CONFIG_LDCONFIG_FILE_SPEC)); do \
		dir=`dirname $$$$file` ; \
		$(INSTALL_DIR) $(1)/$$$$dir ; \
		$(CP) $(call qstrip,$(CONFIG_LDCONFIG_ROOT_DIR))/$$$$file $(1)/$$$$dir/ ; \
	done ; \
	exit 0
  endef

endif

$(eval $(call BuildPackage,libc))
$(eval $(call BuildPackage,libgcc))
$(eval $(call BuildPackage,libatomic))
$(eval $(call BuildPackage,libssp))
$(eval $(call BuildPackage,libstdcpp))
$(eval $(call BuildPackage,libpthread))
$(eval $(call BuildPackage,libthread-db))
$(eval $(call BuildPackage,librt))
$(eval $(call BuildPackage,libgfortran))
$(eval $(call BuildPackage,libgomp))
$(eval $(call BuildPackage,ldd))
$(eval $(call BuildPackage,ldconfig))
