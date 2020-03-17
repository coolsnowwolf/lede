#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

ifeq ($(TARGET_BUILD),1)
  PKG_BUILD_DIR:=$(LINUX_DIR)
endif

ifneq ($(filter host-refresh refresh,$(MAKECMDGOALS)),)
  override QUILT=1
  override HOST_QUILT=1
endif

ifneq ($(PKG_BUILD_DIR),)
  QUILT?=$(if $(wildcard $(PKG_BUILD_DIR)/.quilt_used),y)
  ifneq ($(QUILT),)
    STAMP_CHECKED:=$(PKG_BUILD_DIR)/.quilt_checked
    override CONFIG_AUTOREBUILD=
    override CONFIG_AUTOREMOVE=
    quilt-check: $(STAMP_CHECKED)
  endif
endif

ifneq ($(HOST_BUILD_DIR),)
  HOST_QUILT?=$(if $(findstring command,$(origin QUILT)),$(QUILT),$(if $(wildcard $(HOST_BUILD_DIR)/.quilt_used),y))
  ifneq ($(HOST_QUILT),)
    HOST_STAMP_CHECKED:=$(HOST_BUILD_DIR)/.quilt_checked
    override CONFIG_AUTOREBUILD=
    override CONFIG_AUTOREMOVE=
    host-quilt-check: $(HOST_STAMP_CHECKED)
  endif
endif

ifneq ($(if $(DUMP),1,$(__quilt_inc)),1)
__quilt_inc:=1

PATCH_DIR?=./patches
FILES_DIR?=./files
HOST_PATCH_DIR?=$(PATCH_DIR)
HOST_FILES_DIR?=$(FILES_DIR)

QUILT_CMD:=quilt --quiltrc=-

define filter_series
sed -e s,\\\#.*,, $(1) | grep -E \[a-zA-Z0-9\]
endef

define PatchDir/Quilt
	@mkdir -p "$(1)/patches$(if $(3),/$(patsubst %/,%,$(3)))"
	@if [ -s "$(2)/series" ]; then \
		mkdir -p "$(1)/patches/$(3)"; \
		cp "$(2)/series" "$(1)/patches/$(3)"; \
	fi
	@for patch in $$$$( (cd "$(2)" && if [ -f series ]; then $(call filter_series,series); else ls | sort; fi; ) 2>/dev/null ); do ( \
		cp "$(2)/$$$$patch" "$(1)/patches/$(3)"; \
		echo "$(3)$$$$patch" >> "$(1)/patches/series"; \
	); done
	$(if $(3),@echo $(3) >> "$(1)/patches/.subdirs")
endef

define PatchDir/Default
	@if [ -d "$(2)" ] && [ "$$$$(ls $(2) | wc -l)" -gt 0 ]; then \
		export PATCH="$(PATCH)"; \
		if [ -s "$(2)/series" ]; then \
			$(call filter_series,$(2)/series) | xargs -n1 \
				$(KPATCH) "$(1)" "$(2)"; \
		else \
			$(KPATCH) "$(1)" "$(2)"; \
		fi; \
	fi
endef

define PatchDir
$(call PatchDir/$(if $(strip $(QUILT)),Quilt,Default),$(strip $(1)),$(strip $(2)),$(strip $(3)))
endef

define HostPatchDir
$(call PatchDir/$(if $(strip $(HOST_QUILT)),Quilt,Default),$(strip $(1)),$(strip $(2)),$(strip $(3)))
endef

define Host/Patch/Default
	$(if $(HOST_QUILT),rm -rf $(HOST_BUILD_DIR)/patches; mkdir -p $(HOST_BUILD_DIR)/patches)
	$(call HostPatchDir,$(HOST_BUILD_DIR),$(HOST_PATCH_DIR),)
	$(if $(HOST_QUILT),touch $(HOST_BUILD_DIR)/.quilt_used)
endef

define Build/Patch/Default
	$(if $(QUILT),rm -rf $(PKG_BUILD_DIR)/patches; mkdir -p $(PKG_BUILD_DIR)/patches)
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR),)
	$(if $(QUILT),touch $(PKG_BUILD_DIR)/.quilt_used)
endef

kernel_files=$(foreach fdir,$(GENERIC_FILES_DIR) $(FILES_DIR),$(fdir)/.)
define Kernel/Patch/Default
	$(if $(QUILT),rm -rf $(LINUX_DIR)/patches; mkdir -p $(LINUX_DIR)/patches)
	$(if $(kernel_files),$(CP) $(kernel_files) $(LINUX_DIR)/)
	find $(LINUX_DIR)/ -name \*.rej -or -name \*.orig | $(XARGS) rm -f
	if [ -d $(GENERIC_PLATFORM_DIR)/patches$(if $(wildcard $(GENERIC_PLATFORM_DIR)/patches-$(KERNEL_PATCHVER)),-$(KERNEL_PATCHVER)) ]; then \
		echo "generic patches directory is present. please move your patches to the pending directory" ; \
		exit 1; \
	fi
	$(call PatchDir,$(LINUX_DIR),$(GENERIC_BACKPORT_DIR),generic-backport/)
	$(call PatchDir,$(LINUX_DIR),$(GENERIC_PATCH_DIR),generic/)
	$(call PatchDir,$(LINUX_DIR),$(GENERIC_HACK_DIR),generic-hack/)
	$(call PatchDir,$(LINUX_DIR),$(PATCH_DIR),platform/)
endef

define Quilt/RefreshDir
	mkdir -p $(2)
	-rm -f $(2)/* 2>/dev/null >/dev/null
	@( \
		for patch in $$$$($(if $(3),grep "^$(3)",cat) $(1)/patches/series | awk '{print $$$$1}'); do \
			$(CP) -v "$(1)/patches/$$$$patch" $(2); \
		done; \
	)
endef

define Quilt/Refresh/Host
	$(call Quilt/RefreshDir,$(HOST_BUILD_DIR),$(PATCH_DIR))
endef

define Quilt/Refresh/Package
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR))
endef

define Quilt/Refresh/Kernel
	@[ -z "$$(grep -v '^generic/' $(PKG_BUILD_DIR)/patches/series | grep -v '^platform/')" ] || { \
		echo "All kernel patches must start with either generic/ or platform/"; \
		false; \
	}
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(GENERIC_BACKPORT_DIR),generic-backport/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(GENERIC_PATCH_DIR),generic/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(GENERIC_HACK_DIR),generic-hack/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR),platform/)
endef

define Quilt/Template
  $($(2)STAMP_CONFIGURED): $($(2)STAMP_CHECKED)
  $(if $(NO_RECONFIGURE),$($(2)STAMP_BUILT),$($(2)STAMP_CONFIGURED)): FORCE
  $($(2)STAMP_CHECKED): $($(2)STAMP_PREPARED)
	if [ -s "$(1)/patches/series" ]; then \
		(cd "$(1)"; \
			if $(QUILT_CMD) next >/dev/null 2>&1; then \
				$(QUILT_CMD) push -a; \
			else \
				$(QUILT_CMD) top >/dev/null 2>&1; \
			fi \
		); \
	fi
	touch "$$@"

  $(3)quilt-check: $($(2)STAMP_PREPARED) FORCE
	@[ -f "$(1)/.quilt_used" ] || { \
		echo "The source directory was not unpacked using quilt. Please rebuild with QUILT=1"; \
		false; \
	}
	@[ -f "$(1)/patches/series" ] || { \
		echo "The source directory contains no quilt patches."; \
		false; \
	}
	@[ -n "$$$$(ls $(1)/patches/series)" -o \
	   "$$$$(cat $(1)/patches/series | mkhash md5)" = "$$(sort $(1)/patches/series | mkhash md5)" ] || { \
		echo "The patches are not sorted in the right order. Please fix."; \
		false; \
	}

  $(3)refresh: $(3)quilt-check
	@cd "$(1)"; $(QUILT_CMD) pop -a -f >/dev/null 2>/dev/null
	@cd "$(1)"; while $(QUILT_CMD) next 2>/dev/null >/dev/null && $(QUILT_CMD) push; do \
		QUILT_DIFF_OPTS="-p" $(QUILT_CMD) refresh -p ab --no-index --no-timestamps; \
	done; ! $(QUILT_CMD) next 2>/dev/null >/dev/null
	$(Quilt/Refresh/$(4))
	
  $(3)update: $(3)quilt-check
	$(Quilt/Refresh/$(4))
endef

Build/Quilt=$(call Quilt/Template,$(PKG_BUILD_DIR),,,$(if $(TARGET_BUILD),Kernel,Package))
Host/Quilt=$(call Quilt/Template,$(HOST_BUILD_DIR),HOST_,host-,Host)

endif
