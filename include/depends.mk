#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# define a dependency on a subtree
# parameters:
#	1: directories/files
#	2: directory dependency
#	3: tempfile for file listings
#	4: find options

DEP_FINDPARAMS := -x "*/.svn*" -x ".*" -x "*:*" -x "*\!*" -x "* *" -x "*\\\#*" -x "*/.*_check" -x "*/.*.swp" -x "*/.pkgdir*"

find_md5=find $(wildcard $(1)) -type f $(patsubst -x,-and -not -path,$(DEP_FINDPARAMS) $(2)) | mkhash md5

define rdep
  .PRECIOUS: $(2)
  .SILENT: $(2)_check

  $(2): $(2)_check
  check-depends: $(2)_check

ifneq ($(wildcard $(2)),)
  $(2)_check::
	$(if $(3), \
		$(call find_md5,$(1),$(4)) > $(3).1; \
		{ [ \! -f "$(3)" ] || diff $(3) $(3).1 >/dev/null; } && \
	) \
	{ \
		[ -f "$(2)_check.1" ] && mv "$(2)_check.1"; \
	    $(TOPDIR)/scripts/timestamp.pl $(DEP_FINDPARAMS) $(4) -n $(2) $(1) && { \
			$(call debug_eval,$(SUBDIR),r,echo "No need to rebuild $(2)";) \
			touch -r "$(2)" "$(2)_check"; \
		} \
	} || { \
		$(call debug_eval,$(SUBDIR),r,echo "Need to rebuild $(2)";) \
		touch "$(2)_check"; \
	}
	$(if $(3), mv $(3).1 $(3))
else
  $(2)_check::
	$(if $(3), rm -f $(3) $(3).1)
	$(call debug_eval,$(SUBDIR),r,echo "Target $(2) not built")
endif

endef

ifeq ($(filter .%,$(MAKECMDGOALS)),$(if $(MAKECMDGOALS),$(MAKECMDGOALS),x))
  define rdep
    $(2): $(2)_check
  endef
endif
