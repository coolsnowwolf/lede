#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifneq ($(__prereq_inc),1)
__prereq_inc:=1

prereq:
	if [ -f $(TMP_DIR)/.prereq-error ]; then \
		echo; \
		cat $(TMP_DIR)/.prereq-error; \
		rm -f $(TMP_DIR)/.prereq-error; \
		echo; \
		false; \
	fi

.SILENT: prereq
endif

PREREQ_PREV=

# 1: display name
# 2: error message
define Require
  export PREREQ_CHECK=1
  ifeq ($$(CHECK_$(1)),)
    prereq: prereq-$(1)

    prereq-$(1): $(if $(PREREQ_PREV),prereq-$(PREREQ_PREV)) FORCE
		printf "Checking '$(1)'... "
		if $(NO_TRACE_MAKE) -f $(firstword $(MAKEFILE_LIST)) check-$(1) >/dev/null 2>/dev/null; then \
			echo 'ok.'; \
		else \
			echo 'failed.'; \
			echo "$(PKG_NAME): $(strip $(2))" >> $(TMP_DIR)/.prereq-error; \
		fi

    check-$(1): FORCE
	  $(call Require/$(1))
    CHECK_$(1):=1

    .SILENT: prereq-$(1) check-$(1)
    .NOTPARALLEL:
  endif

  PREREQ_PREV=$(1)
endef


define RequireCommand
  define Require/$(1)
    which $(1)
  endef

  $$(eval $$(call Require,$(1),$(2)))
endef

define RequireHeader
  define Require/$(1)
    [ -e "$(1)" ]
  endef

  $$(eval $$(call Require,$(1),$(2)))
endef

define QuoteHostCommand
'$(subst ','"'"',$(strip $(1)))'
endef

# 1: display name
# 2: failure message
# 3: test
define TestHostCommand
  define Require/$(1)
	($(3)) >/dev/null 2>/dev/null
  endef

  $$(eval $$(call Require,$(1),$(2)))
endef

# 1: canonical name
# 2: failure message
# 3+: candidates
define SetupHostCommand
  define Require/$(1)
	[ -f "$(STAGING_DIR_HOST)/bin/$(strip $(1))" ] && exit 0; \
	for cmd in $(call QuoteHostCommand,$(3)) $(call QuoteHostCommand,$(4)) \
	           $(call QuoteHostCommand,$(5)) $(call QuoteHostCommand,$(6)) \
	           $(call QuoteHostCommand,$(7)) $(call QuoteHostCommand,$(8)) \
	           $(call QuoteHostCommand,$(9)) $(call QuoteHostCommand,$(10)) \
	           $(call QuoteHostCommand,$(11)) $(call QuoteHostCommand,$(12)); do \
		if [ -n "$$$$$$$$cmd" ]; then \
			bin="$$$$$$$$(PATH="$(subst $(space),:,$(filter-out $(STAGING_DIR_HOST)/%,$(subst :,$(space),$(PATH))))" \
				which "$$$$$$$${cmd%% *}")"; \
			if [ -x "$$$$$$$$bin" ] && eval "$$$$$$$$cmd" >/dev/null 2>/dev/null; then \
				mkdir -p "$(STAGING_DIR_HOST)/bin"; \
				ln -sf "$$$$$$$$bin" "$(STAGING_DIR_HOST)/bin/$(strip $(1))"; \
				exit 0; \
			fi; \
		fi; \
	done; \
	exit 1
  endef

  $$(eval $$(call Require,$(1),$(if $(2),$(2),Missing $(1) command)))
endef
