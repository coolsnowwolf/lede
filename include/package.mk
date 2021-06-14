# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2020 OpenWrt.org

__package_mk:=1

all: $(if $(DUMP),dumpinfo,$(if $(CHECK),check,compile))

include $(INCLUDE_DIR)/download.mk

PKG_BUILD_DIR ?= $(BUILD_DIR)/$(if $(BUILD_VARIANT),$(PKG_NAME)-$(BUILD_VARIANT)/)$(PKG_NAME)$(if $(PKG_VERSION),-$(PKG_VERSION))
PKG_INSTALL_DIR ?= $(PKG_BUILD_DIR)/ipkg-install
PKG_BUILD_PARALLEL ?=
PKG_USE_MIPS16 ?= 1
PKG_IREMAP ?= 1

MAKE_J:=$(if $(MAKE_JOBSERVER),$(MAKE_JOBSERVER) $(if $(filter 3.% 4.0 4.1,$(MAKE_VERSION)),-j))

PKG_SOURCE_DATE_EPOCH:=$(if $(DUMP),,$(shell $(TOPDIR)/scripts/get_source_date_epoch.sh $(CURDIR)))

ifeq ($(strip $(PKG_BUILD_PARALLEL)),0)
PKG_JOBS?=-j1
else
PKG_JOBS?=$(if $(PKG_BUILD_PARALLEL),$(MAKE_J),-j1)
endif
ifdef CONFIG_USE_MIPS16
  ifeq ($(strip $(PKG_USE_MIPS16)),1)
    TARGET_ASFLAGS_DEFAULT = $(filter-out -mips16 -minterlink-mips16,$(TARGET_CFLAGS))
    TARGET_CFLAGS += -mips16 -minterlink-mips16
  endif
endif
ifeq ($(strip $(PKG_IREMAP)),1)
  IREMAP_CFLAGS = $(call iremap,$(PKG_BUILD_DIR),$(notdir $(PKG_BUILD_DIR)))
  TARGET_CFLAGS += $(IREMAP_CFLAGS)
endif

include $(INCLUDE_DIR)/hardening.mk
include $(INCLUDE_DIR)/prereq.mk
include $(INCLUDE_DIR)/unpack.mk
include $(INCLUDE_DIR)/depends.mk

ifneq ($(wildcard $(TOPDIR)/git-src/$(PKG_NAME)/.git),)
  USE_GIT_SRC_CHECKOUT:=1
  QUILT:=1
endif
ifneq ($(if $(CONFIG_SRC_TREE_OVERRIDE),$(wildcard ./git-src)),)
  USE_GIT_TREE:=1
  QUILT:=1
endif
ifdef USE_SOURCE_DIR
  QUILT:=1
endif
ifneq ($(wildcard $(PKG_BUILD_DIR)/.source_dir),)
  QUILT:=1
endif

include $(INCLUDE_DIR)/quilt.mk

find_library_dependencies = \
	$(wildcard $(patsubst %,$(STAGING_DIR)/pkginfo/%.version, \
		$(filter-out $(BUILD_PACKAGES), $(sort $(foreach dep4, \
			$(sort $(foreach dep3, \
				$(sort $(foreach dep2, \
					$(sort $(foreach dep1, \
						$(sort $(foreach dep0, \
							$(Package/$(1)/depends), \
							$(Package/$(dep0)/depends) $(dep0) \
						)), \
						$(Package/$(dep1)/depends) $(dep1) \
					)), \
					$(Package/$(dep2)/depends) $(dep2) \
				)), \
				$(Package/$(dep3)/depends) $(dep3) \
			)), \
			$(Package/$(dep4)/depends) $(dep4) \
		))) \
	))


PKG_DIR_NAME:=$(lastword $(subst /,$(space),$(CURDIR)))
STAMP_NO_AUTOREBUILD=$(wildcard $(PKG_BUILD_DIR)/.no_autorebuild)
PREV_STAMP_PREPARED:=$(if $(STAMP_NO_AUTOREBUILD),$(wildcard $(PKG_BUILD_DIR)/.prepared*))
ifneq ($(PREV_STAMP_PREPARED),)
  STAMP_PREPARED:=$(PREV_STAMP_PREPARED)
  CONFIG_AUTOREBUILD:=
else
  STAMP_PREPARED=$(PKG_BUILD_DIR)/.prepared$(if $(QUILT)$(DUMP),,_$(shell $(call find_md5,${CURDIR} $(PKG_FILE_DEPENDS),))_$(call confvar,CONFIG_AUTOREMOVE $(PKG_PREPARED_DEPENDS)))
endif
STAMP_CONFIGURED=$(PKG_BUILD_DIR)/.configured$(if $(DUMP),,_$(call confvar,$(PKG_CONFIG_DEPENDS)))
STAMP_CONFIGURED_WILDCARD=$(PKG_BUILD_DIR)/.configured_*
STAMP_BUILT:=$(PKG_BUILD_DIR)/.built
STAMP_INSTALLED:=$(STAGING_DIR)/stamp/.$(PKG_DIR_NAME)$(if $(BUILD_VARIANT),.$(BUILD_VARIANT),)_installed

STAGING_FILES_LIST:=$(PKG_DIR_NAME)$(if $(BUILD_VARIANT),.$(BUILD_VARIANT),).list

define CleanStaging
	rm -f $(STAMP_INSTALLED)
	@-(\
		if [ -f $(STAGING_DIR)/packages/$(STAGING_FILES_LIST) ]; then \
			$(SCRIPT_DIR)/clean-package.sh \
				"$(STAGING_DIR)/packages/$(STAGING_FILES_LIST)" \
				"$(STAGING_DIR)"; \
		fi; \
	)
endef


PKG_INSTALL_STAMP:=$(PKG_INFO_DIR)/$(PKG_DIR_NAME).$(if $(BUILD_VARIANT),$(BUILD_VARIANT),default).install

include $(INCLUDE_DIR)/package-defaults.mk
include $(INCLUDE_DIR)/package-dumpinfo.mk
include $(INCLUDE_DIR)/package-ipkg.mk
include $(INCLUDE_DIR)/package-bin.mk
include $(INCLUDE_DIR)/autotools.mk

_pkg_target:=$(if $(QUILT),,.)

override MAKEFLAGS=
CONFIG_SITE:=$(INCLUDE_DIR)/site/$(ARCH)
CUR_MAKEFILE:=$(filter-out Makefile,$(firstword $(MAKEFILE_LIST)))
SUBMAKE:=$(NO_TRACE_MAKE) $(if $(CUR_MAKEFILE),-f $(CUR_MAKEFILE))
PKG_CONFIG_PATH=$(STAGING_DIR)/usr/lib/pkgconfig:$(STAGING_DIR)/usr/share/pkgconfig
unexport QUIET CONFIG_SITE

ifeq ($(DUMP)$(filter prereq clean refresh update,$(MAKECMDGOALS)),)
  ifneq ($(if $(QUILT),,$(CONFIG_AUTOREBUILD)),)
    define Build/Autoclean
      $(PKG_BUILD_DIR)/.dep_files: $(STAMP_PREPARED)
      $(call rdep,${CURDIR} $(PKG_FILE_DEPENDS),$(STAMP_PREPARED),$(PKG_BUILD_DIR)/.dep_files,-x "*/.dep_*")
      $(if $(filter prepare,$(MAKECMDGOALS)),,$(call rdep,$(PKG_BUILD_DIR),$(STAMP_BUILT),,-x "*/.dep_*" -x "*/ipkg*"))
    endef
  endif
endif

ifdef USE_GIT_SRC_CHECKOUT
  define Build/Prepare/Default
	mkdir -p $(PKG_BUILD_DIR)
	ln -s $(TOPDIR)/git-src/$(PKG_NAME)/.git $(PKG_BUILD_DIR)/.git
	( cd $(PKG_BUILD_DIR); \
		git checkout .; \
		git submodule update --recursive; \
		git submodule foreach git config --unset core.worktree; \
		git submodule foreach git checkout .; \
	)
  endef
endif
ifdef USE_GIT_TREE
  define Build/Prepare/Default
	mkdir -p $(PKG_BUILD_DIR)
	ln -s $(CURDIR)/git-src $(PKG_BUILD_DIR)/.git
	( cd $(PKG_BUILD_DIR); \
		git checkout .; \
		git submodule update --recursive; \
		git submodule foreach git config --unset core.worktree; \
		git submodule foreach git checkout .; \
	)
  endef
endif
ifdef USE_SOURCE_DIR
  define Build/Prepare/Default
	rm -rf $(PKG_BUILD_DIR)
	$(if $(wildcard $(USE_SOURCE_DIR)/*),,@echo "Error: USE_SOURCE_DIR=$(USE_SOURCE_DIR) path not found"; false)
	ln -snf $(USE_SOURCE_DIR) $(PKG_BUILD_DIR)
	touch $(PKG_BUILD_DIR)/.source_dir
  endef
endif

define Build/Exports/Default
  $(1) : export ACLOCAL_INCLUDE=$$(foreach p,$$(wildcard $$(STAGING_DIR)/usr/share/aclocal $$(STAGING_DIR)/usr/share/aclocal-* $$(STAGING_DIR_HOSTPKG)/share/aclocal $$(STAGING_DIR_HOSTPKG)/share/aclocal-* $$(STAGING_DIR)/host/share/aclocal $$(STAGING_DIR)/host/share/aclocal-*),-I $$(p))
  $(1) : export STAGING_PREFIX=$$(STAGING_DIR)/usr
  $(1) : export PATH=$$(TARGET_PATH_PKG)
  $(1) : export CONFIG_SITE:=$$(CONFIG_SITE)
  $(1) : export PKG_CONFIG_PATH:=$$(PKG_CONFIG_PATH)
  $(1) : export PKG_CONFIG_LIBDIR:=$$(PKG_CONFIG_PATH)
endef
Build/Exports=$(Build/Exports/Default)

define Build/CoreTargets
  STAMP_PREPARED:=$$(STAMP_PREPARED)
  STAMP_CONFIGURED:=$$(STAMP_CONFIGURED)

  $(if $(QUILT),$(Build/Quilt))
  $(call Build/Autoclean)
  $(call DefaultTargets)

  $(DL_DIR)/$(FILE): FORCE

  download:
	$(foreach hook,$(Hooks/Download),
		$(call $(hook))$(sep)
	)

  $(STAMP_PREPARED) : export PATH=$$(TARGET_PATH_PKG)
  $(STAMP_PREPARED): $(STAMP_PREPARED_DEPENDS)
	@-rm -rf $(PKG_BUILD_DIR)
	@mkdir -p $(PKG_BUILD_DIR)
	touch $$@_check
	$(foreach hook,$(Hooks/Prepare/Pre),$(call $(hook))$(sep))
	$(Build/Prepare)
	$(foreach hook,$(Hooks/Prepare/Post),$(call $(hook))$(sep))
	touch $$@

  $(call Build/Exports,$(STAMP_CONFIGURED))
  $(STAMP_CONFIGURED): $(STAMP_PREPARED) $(STAMP_CONFIGURED_DEPENDS)
	rm -f $(STAMP_CONFIGURED_WILDCARD)
	$(CleanStaging)
	$(foreach hook,$(Hooks/Configure/Pre),$(call $(hook))$(sep))
	$(Build/Configure)
	$(foreach hook,$(Hooks/Configure/Post),$(call $(hook))$(sep))
	touch $$@

  $(call Build/Exports,$(STAMP_BUILT))
  $(STAMP_BUILT): $(STAMP_CONFIGURED) $(STAMP_BUILT_DEPENDS)
	rm -f $$@
	touch $$@_check
	$(foreach hook,$(Hooks/Compile/Pre),$(call $(hook))$(sep))
	$(Build/Compile)
	$(foreach hook,$(Hooks/Compile/Post),$(call $(hook))$(sep))
	$(Build/Install)
	$(foreach hook,$(Hooks/Install/Post),$(call $(hook))$(sep))
	touch $$@

  $(STAMP_INSTALLED) : export PATH=$$(TARGET_PATH_PKG)
  $(STAMP_INSTALLED): $(STAMP_BUILT)
	rm -rf $(TMP_DIR)/stage-$(PKG_DIR_NAME)
	mkdir -p $(TMP_DIR)/stage-$(PKG_DIR_NAME)/host $(STAGING_DIR)/packages $(STAGING_DIR_HOST)/packages
	$(foreach hook,$(Hooks/InstallDev/Pre),\
		$(call $(hook),$(TMP_DIR)/stage-$(PKG_DIR_NAME),$(TMP_DIR)/stage-$(PKG_DIR_NAME)/host)$(sep)\
	)
	$(call Build/InstallDev,$(TMP_DIR)/stage-$(PKG_DIR_NAME),$(TMP_DIR)/stage-$(PKG_DIR_NAME)/host)
	$(foreach hook,$(Hooks/InstallDev/Post),\
		$(call $(hook),$(TMP_DIR)/stage-$(PKG_DIR_NAME),$(TMP_DIR)/stage-$(PKG_DIR_NAME)/host)$(sep)\
	)
	if [ -f $(STAGING_DIR)/packages/$(STAGING_FILES_LIST) ]; then \
		$(SCRIPT_DIR)/clean-package.sh \
			"$(STAGING_DIR)/packages/$(STAGING_FILES_LIST)" \
			"$(STAGING_DIR)"; \
	fi
	if [ -d $(TMP_DIR)/stage-$(PKG_DIR_NAME) ]; then \
		(cd $(TMP_DIR)/stage-$(PKG_DIR_NAME); find ./ > $(TMP_DIR)/stage-$(PKG_DIR_NAME).files); \
		$(call locked, \
			mv $(TMP_DIR)/stage-$(PKG_DIR_NAME).files $(STAGING_DIR)/packages/$(STAGING_FILES_LIST) && \
			$(CP) $(TMP_DIR)/stage-$(PKG_DIR_NAME)/* $(STAGING_DIR)/; \
		,staging-dir); \
	fi
	rm -rf $(TMP_DIR)/stage-$(PKG_DIR_NAME)
	touch $$@

  ifdef Build/InstallDev
    $(_pkg_target)compile: $(STAMP_INSTALLED)
  endif

  $(_pkg_target)prepare: $(STAMP_PREPARED)
  $(_pkg_target)configure: $(STAMP_CONFIGURED)
  $(_pkg_target)dist: $(STAMP_CONFIGURED)
  $(_pkg_target)distcheck: $(STAMP_CONFIGURED)

  ifneq ($(CONFIG_AUTOREMOVE),)
    compile:
		-touch -r $(PKG_BUILD_DIR)/.built $(PKG_BUILD_DIR)/.autoremove 2>/dev/null >/dev/null
		$(FIND) $(PKG_BUILD_DIR) -mindepth 1 -maxdepth 1 -not '(' -type f -and -name '.*' -and -size 0 ')' -and -not -name '.pkgdir' | \
			$(XARGS) rm -rf
  endif
endef

define Build/DefaultTargets
  $(if $(USE_SOURCE_DIR)$(USE_GIT_TREE)$(USE_GIT_SRC_CHECKOUT),,$(if $(strip $(PKG_SOURCE_URL)),$(call Download,default)))
  $(if $(DUMP),,$(Build/CoreTargets))

  define Build/DefaultTargets
  endef
endef

define BuildPackage
  $(eval $(Package/Default))
  $(eval $(Package/$(1)))

ifdef DESCRIPTION
$$(error DESCRIPTION:= is obsolete, use Package/PKG_NAME/description)
endif

ifndef Package/$(1)/description
define Package/$(1)/description
	$(TITLE)
endef
endif

  BUILD_PACKAGES += $(1)
  $(STAMP_PREPARED): $$(if $(QUILT)$(DUMP),,$(call find_library_dependencies,$(1)))

  $(foreach FIELD, TITLE CATEGORY SECTION VERSION,
    ifeq ($($(FIELD)),)
      $$(error Package/$(1) is missing the $(FIELD) field)
    endif
  )

  $(if $(DUMP), \
    $(if $(CHECK),,$(Dumpinfo/Package)), \
    $(foreach target, \
      $(if $(Package/$(1)/targets),$(Package/$(1)/targets), \
        $(if $(PKG_TARGETS),$(PKG_TARGETS), ipkg) \
      ), $(BuildTarget/$(target)) \
    ) \
  )
  $(if $(PKG_HOST_ONLY),,$(call Build/DefaultTargets,$(1)))
endef

define pkg_install_files
	$(foreach install_file,$(1),$(INSTALL_DIR) $(3)/`dirname $(install_file)`; $(INSTALL_DATA) $(2)/$(install_file) $(3)/`dirname $(install_file)`;)
endef

define pkg_install_bin
	$(foreach install_apps,$(1),$(INSTALL_DIR) $(3)/`dirname $(install_apps)`; $(INSTALL_BIN) $(2)/$(install_apps) $(3)/`dirname $(install_apps)`;)
endef

Build/Prepare=$(call Build/Prepare/Default,)
Build/Configure=$(call Build/Configure/Default,)
Build/Compile=$(call Build/Compile/Default,)
Build/Install=$(if $(PKG_INSTALL),$(call Build/Install/Default,))
Build/Dist=$(call Build/Dist/Default,)
Build/DistCheck=$(call Build/DistCheck/Default,)

.NOTPARALLEL:

.PHONY: prepare-package-install
prepare-package-install:
	@mkdir -p $(PKG_INFO_DIR)
	@rm -f $(PKG_INSTALL_STAMP)
	@echo "$(filter-out essential nonshared,$(PKG_FLAGS))" > $(PKG_INSTALL_STAMP).flags

$(PACKAGE_DIR):
	mkdir -p $@

compile:
.install: .compile
install: compile

force-clean-build: FORCE
	rm -rf $(PKG_BUILD_DIR)

clean-build: $(if $(wildcard $(PKG_BUILD_DIR)/.autoremove),force-clean-build)

clean: force-clean-build
	$(CleanStaging)
	$(call Build/UninstallDev,$(STAGING_DIR),$(STAGING_DIR_HOST))
	$(Build/Clean)
	rm -f $(STAGING_DIR)/packages/$(STAGING_FILES_LIST) $(STAGING_DIR_HOST)/packages/$(STAGING_FILES_LIST)

dist:
	$(Build/Dist)

distcheck:
	$(Build/DistCheck)
