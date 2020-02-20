#
# Copyright (C) 2006-2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(INCLUDE_DIR)/prereq.mk
include $(INCLUDE_DIR)/depends.mk

ifneq ($(DUMP),1)
  all: compile
endif

KERNEL_FILE_DEPENDS=$(GENERIC_BACKPORT_DIR) $(GENERIC_PATCH_DIR) $(GENERIC_HACK_DIR) $(PATCH_DIR) $(GENERIC_FILES_DIR) $(FILES_DIR)
STAMP_PREPARED=$(LINUX_DIR)/.prepared$(if $(QUILT)$(DUMP),,_$(shell $(call find_md5,$(KERNEL_FILE_DEPENDS),)))
STAMP_CONFIGURED:=$(LINUX_DIR)/.configured
include $(INCLUDE_DIR)/download.mk
include $(INCLUDE_DIR)/quilt.mk
include $(INCLUDE_DIR)/kernel-defaults.mk

define Kernel/Prepare
	$(call Kernel/Prepare/Default)
endef

define Kernel/Configure
	$(call Kernel/Configure/Default)
endef

define Kernel/CompileModules
	$(call Kernel/CompileModules/Default)
endef

define Kernel/CompileImage
	$(call Kernel/CompileImage/Default)
	$(call Kernel/CompileImage/Initramfs)
endef

define Kernel/Clean
	$(call Kernel/Clean/Default)
endef

define Download/kernel
  URL:=$(LINUX_SITE)
  FILE:=$(LINUX_SOURCE)
  HASH:=$(LINUX_KERNEL_HASH)
endef

KERNEL_GIT_OPTS:=
ifneq ($(strip $(CONFIG_KERNEL_GIT_LOCAL_REPOSITORY)),"")
  KERNEL_GIT_OPTS+=--reference $(CONFIG_KERNEL_GIT_LOCAL_REPOSITORY)
endif

define Download/git-kernel
  URL:=$(call qstrip,$(CONFIG_KERNEL_GIT_CLONE_URI))
  PROTO:=git
  VERSION:=$(CONFIG_KERNEL_GIT_REF)
  FILE:=$(LINUX_SOURCE)
  SUBDIR:=linux-$(LINUX_VERSION)
  OPTS:=$(KERNEL_GIT_OPTS)
endef

ifdef CONFIG_COLLECT_KERNEL_DEBUG
  define Kernel/CollectDebug
	rm -rf $(KERNEL_BUILD_DIR)/debug
	mkdir -p $(KERNEL_BUILD_DIR)/debug/modules
	$(CP) $(LINUX_DIR)/vmlinux $(KERNEL_BUILD_DIR)/debug/
	-$(CP) \
		$(STAGING_DIR_ROOT)/lib/modules/$(LINUX_VERSION)/* \
		$(KERNEL_BUILD_DIR)/debug/modules/
	$(FIND) $(KERNEL_BUILD_DIR)/debug -type f | $(XARGS) $(KERNEL_CROSS)strip --only-keep-debug
	$(TAR) c -C $(KERNEL_BUILD_DIR) debug \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		| bzip2 -c -9 > $(BIN_DIR)/kernel-debug.tar.bz2
  endef
endif

ifeq ($(DUMP)$(filter prereq clean refresh update,$(MAKECMDGOALS)),)
  ifneq ($(if $(QUILT),,$(CONFIG_AUTOREBUILD)),)
    define Kernel/Autoclean
      $(PKG_BUILD_DIR)/.dep_files: $(STAMP_PREPARED)
      $(call rdep,$(KERNEL_FILE_DEPENDS),$(STAMP_PREPARED),$(PKG_BUILD_DIR)/.dep_files,-x "*/.dep_*")
    endef
  endif
endif

define BuildKernel
  $(if $(QUILT),$(Build/Quilt))
  $(if $(LINUX_SITE),$(call Download,kernel))
  $(if $(call qstrip,$(CONFIG_KERNEL_GIT_CLONE_URI)),$(call Download,git-kernel))

  .NOTPARALLEL:

  $(Kernel/Autoclean)
  $(STAMP_PREPARED): $(if $(LINUX_SITE),$(DL_DIR)/$(LINUX_SOURCE))
	-rm -rf $(KERNEL_BUILD_DIR)
	-mkdir -p $(KERNEL_BUILD_DIR)
	$(Kernel/Prepare)
	touch $$@

  $(KERNEL_BUILD_DIR)/symtab.h: FORCE
	rm -f $(KERNEL_BUILD_DIR)/symtab.h
	touch $(KERNEL_BUILD_DIR)/symtab.h
	+$(KERNEL_MAKE) vmlinux
	find $(LINUX_DIR) $(STAGING_DIR_ROOT)/lib/modules -name \*.ko | \
		xargs $(TARGET_CROSS)nm | \
		awk '$$$$1 == "U" { print $$$$2 } ' | \
		sort -u > $(KERNEL_BUILD_DIR)/mod_symtab.txt
	$(TARGET_CROSS)nm -n $(LINUX_DIR)/vmlinux.o | grep ' [rR] __ksymtab' | sed -e 's,........ [rR] __ksymtab_,,' > $(KERNEL_BUILD_DIR)/kernel_symtab.txt
	grep -Ff $(KERNEL_BUILD_DIR)/mod_symtab.txt $(KERNEL_BUILD_DIR)/kernel_symtab.txt > $(KERNEL_BUILD_DIR)/sym_include.txt
	grep -Fvf $(KERNEL_BUILD_DIR)/mod_symtab.txt $(KERNEL_BUILD_DIR)/kernel_symtab.txt > $(KERNEL_BUILD_DIR)/sym_exclude.txt
	( \
		echo '#define SYMTAB_KEEP \'; \
		cat $(KERNEL_BUILD_DIR)/sym_include.txt | \
			awk '{print "KEEP(*(___ksymtab+" $$$$1 ")) \\" }'; \
		echo; \
		echo '#define SYMTAB_KEEP_GPL \'; \
		cat $(KERNEL_BUILD_DIR)/sym_include.txt | \
			awk '{print "KEEP(*(___ksymtab_gpl+" $$$$1 ")) \\" }'; \
		echo; \
		echo '#define SYMTAB_DISCARD \'; \
		cat $(KERNEL_BUILD_DIR)/sym_exclude.txt | \
			awk '{print "*(___ksymtab+" $$$$1 ") \\" }'; \
		echo; \
		echo '#define SYMTAB_DISCARD_GPL \'; \
		cat $(KERNEL_BUILD_DIR)/sym_exclude.txt | \
			awk '{print "*(___ksymtab_gpl+" $$$$1 ") \\" }'; \
		echo; \
	) > $$@

  $(STAMP_CONFIGURED): $(STAMP_PREPARED) $(LINUX_KCONFIG_LIST) $(TOPDIR)/.config FORCE
	$(Kernel/Configure)
	touch $$@

  $(LINUX_DIR)/.modules: export STAGING_PREFIX=$$(STAGING_DIR_HOST)
  $(LINUX_DIR)/.modules: export PKG_CONFIG_PATH=$$(STAGING_DIR_HOST)/lib/pkgconfig
  $(LINUX_DIR)/.modules: export PKG_CONFIG_LIBDIR=$$(STAGING_DIR_HOST)/lib/pkgconfig
  $(LINUX_DIR)/.modules: $(STAMP_CONFIGURED) $(LINUX_DIR)/.config FORCE
	$(Kernel/CompileModules)
	touch $$@

  $(LINUX_DIR)/.image: export STAGING_PREFIX=$$(STAGING_DIR_HOST)
  $(LINUX_DIR)/.image: export PKG_CONFIG_PATH=$$(STAGING_DIR_HOST)/lib/pkgconfig
  $(LINUX_DIR)/.image: export PKG_CONFIG_LIBDIR=$$(STAGING_DIR_HOST)/lib/pkgconfig
  $(LINUX_DIR)/.image: $(STAMP_CONFIGURED) $(if $(CONFIG_STRIP_KERNEL_EXPORTS),$(KERNEL_BUILD_DIR)/symtab.h) FORCE
	$(Kernel/CompileImage)
	$(Kernel/CollectDebug)
	touch $$@
	
  mostlyclean: FORCE
	$(Kernel/Clean)

  define BuildKernel
  endef

  download: $(if $(LINUX_SITE),$(DL_DIR)/$(LINUX_SOURCE))
  prepare: $(STAMP_PREPARED)
  compile: $(LINUX_DIR)/.modules
	$(MAKE) -C image compile TARGET_BUILD=

  oldconfig menuconfig nconfig: $(STAMP_PREPARED) $(STAMP_CHECKED) FORCE
	rm -f $(LINUX_DIR)/.config.prev
	rm -f $(STAMP_CONFIGURED)
	$(LINUX_RECONF_CMD) > $(LINUX_DIR)/.config
	$(_SINGLE)$(KERNEL_MAKE) \
		$(if $(findstring Darwin,$(HOST_OS)),HOST_LOADLIBES="-L$(STAGING_DIR_HOST)/lib -lncurses") \
		YACC=$(STAGING_DIR_HOST)/bin/bison \
		$$@
	$(LINUX_RECONF_DIFF) $(LINUX_DIR)/.config | \
		grep -vE '(CONFIG_CC_(HAS_ASM_GOTO|IS_GCC|IS_CLANG)|GCC_VERSION)=' \
		> $(LINUX_RECONFIG_TARGET)

  install: $(LINUX_DIR)/.image
	+$(MAKE) -C image compile install TARGET_BUILD=

  clean: FORCE
	rm -rf $(KERNEL_BUILD_DIR)

  image-prereq:
	@+$(NO_TRACE_MAKE) -s -C image prereq TARGET_BUILD=

  prereq: image-prereq

endef
