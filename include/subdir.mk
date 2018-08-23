#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifeq ($(MAKECMDGOALS),prereq)
  SUBTARGETS:=prereq
  PREREQ_ONLY:=1
else
  SUBTARGETS:=$(DEFAULT_SUBDIR_TARGETS)
endif

subtarget-default = $(filter-out ., \
	$(if $($(1)/builddirs-$(2)),$($(1)/builddirs-$(2)), \
	$(if $($(1)/builddirs-default),$($(1)/builddirs-default), \
	$($(1)/builddirs))))

define subtarget
  $(call warn_eval,$(1),t,T,$(1)/$(2): $($(1)/) $(foreach bd,$(call subtarget-default,$(1),$(2)),$(1)/$(bd)/$(2)))

endef

define ERROR
	($(call MESSAGE, $(2)); $(if $(BUILD_LOG), echo "$(2)" >> $(BUILD_LOG_DIR)/$(1)/error.txt))
endef

lastdir=$(word $(words $(subst /, ,$(1))),$(subst /, ,$(1)))
diralias=$(if $(findstring $(1),$(call lastdir,$(1))),,$(call lastdir,$(1)))

subdir_make_opts = \
	-r -C $(1) \
		BUILD_SUBDIR="$(1)" \
		BUILD_VARIANT="$(4)"

# 1: subdir
# 2: target
# 3: build type
# 4: build variant
log_make = \
	 $(if $(call debug,$(1),v),,@)+ \
	 $(if $(BUILD_LOG), \
		set -o pipefail; \
		mkdir -p $(BUILD_LOG_DIR)/$(1)$(if $(4),/$(4));) \
	env time -f "time: $(1)$(if $(4),/$(4))/$(if $(3),$(3)-)$(2)\#%U\#%S\#%e" -- \
	$$(SUBMAKE) $(subdir_make_opts) $(if $(3),$(3)-)$(2) \
		$(if $(BUILD_LOG),SILENT= 2>&1 | tee $(BUILD_LOG_DIR)/$(1)$(if $(4),/$(4))/$(if $(3),$(3)-)$(2).txt)

ifdef CONFIG_AUTOREMOVE
rebuild_check = \
	@-$$(NO_TRACE_MAKE) $(subdir_make_opts) check-depends >/dev/null 2>/dev/null; \
		$(if $(BUILD_LOG),mkdir -p $(BUILD_LOG_DIR)/$(1)$(if $(4),/$(4));) \
		$$(NO_TRACE_MAKE) $(if $(BUILD_LOG),-d) -q $(subdir_make_opts) .$(if $(3),$(3)-)$(2) \
			> $(if $(BUILD_LOG),$(BUILD_LOG_DIR)/$(1)$(if $(4),/$(4))/check-$(if $(3),$(3)-)$(2).txt,/dev/null) 2>&1 || \
			$$(SUBMAKE) $(subdir_make_opts) clean-build >/dev/null 2>/dev/null

endif

# Parameters: <subdir>
define subdir
  $(call warn,$(1),d,D $(1))
  $(foreach bd,$($(1)/builddirs),
    $(call warn,$(1),d,BD $(1)/$(bd))
    $(foreach target,$(SUBTARGETS) $($(1)/subtargets),
      $(foreach btype,$(buildtypes-$(bd)),
        $(call warn_eval,$(1)/$(bd),t,T,$(1)/$(bd)/$(btype)/$(target): $(if $(NO_DEPS)$(QUILT),,$($(1)/$(bd)/$(btype)/$(target)) $(call $(1)//$(btype)/$(target),$(1)/$(bd)/$(btype))))
		  $(call log_make,$(1)/$(bd),$(target),$(btype),$(filter-out __default,$(variant))) \
			$(if $(findstring $(bd),$($(1)/builddirs-ignore-$(btype)-$(target))), || $(call ERROR,$(1),   ERROR: $(1)/$(bd) [$(btype)] failed to build.))
        $(if $(call diralias,$(bd)),$(call warn_eval,$(1)/$(bd),l,T,$(1)/$(call diralias,$(bd))/$(btype)/$(target): $(1)/$(bd)/$(btype)/$(target)))
      )
      $(call warn_eval,$(1)/$(bd),t,T,$(1)/$(bd)/$(target): $(if $(NO_DEPS)$(QUILT),,$($(1)/$(bd)/$(target)) $(call $(1)//$(target),$(1)/$(bd))))
        $(foreach variant,$(if $(BUILD_VARIANT),$(BUILD_VARIANT),$(if $(strip $($(1)/$(bd)/variants)),$($(1)/$(bd)/variants),$(if $($(1)/$(bd)/default-variant),$($(1)/$(bd)/default-variant),__default))),
			$(if $(BUILD_LOG),@mkdir -p $(BUILD_LOG_DIR)/$(1)/$(bd)/$(filter-out __default,$(variant)))
			$(if $($(1)/autoremove),$(call rebuild_check,$(1)/$(bd),$(target),,$(filter-out __default,$(variant))))
			$(call log_make,$(1)/$(bd),$(target),,$(filter-out __default,$(variant))) \
				$(if $(findstring $(bd),$($(1)/builddirs-ignore-$(target))), || $(call ERROR,$(1),   ERROR: $(1)/$(bd) failed to build$(if $(filter-out __default,$(variant)), (build variant: $(variant))).))
        )
      $(if $(PREREQ_ONLY)$(DUMP_TARGET_DB),,
        # aliases
        $(if $(call diralias,$(bd)),$(call warn_eval,$(1)/$(bd),l,T,$(1)/$(call diralias,$(bd))/$(target): $(1)/$(bd)/$(target)))
	  )
	)
  )
  $(foreach target,$(SUBTARGETS) $($(1)/subtargets),$(call subtarget,$(1),$(target)))
endef

ifndef DUMP_TARGET_DB
# Parameters: <subdir> <name> <target> <depends> <config options> <stampfile location>
define stampfile
  $(1)/stamp-$(3):=$(if $(6),$(6),$(STAGING_DIR))/stamp/.$(2)_$(3)$(5)
  $$($(1)/stamp-$(3)): $(TMP_DIR)/.build $(4)
	@+$(SCRIPT_DIR)/timestamp.pl -n $$($(1)/stamp-$(3)) $(1) $(4) || \
		$(MAKE) $(if $(QUIET),--no-print-directory) $$($(1)/flags-$(3)) $(1)/$(3)
	@mkdir -p $$$$(dirname $$($(1)/stamp-$(3)))
	@touch $$($(1)/stamp-$(3))

  $$(if $(call debug,$(1),v),,.SILENT: $$($(1)/stamp-$(3)))

  .PRECIOUS: $$($(1)/stamp-$(3)) # work around a make bug

  $(1)//clean:=$(1)/stamp-$(3)/clean
  $(1)/stamp-$(3)/clean: FORCE
	@rm -f $$($(1)/stamp-$(3))

endef
endif
