#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifndef DUMP
  include $(INCLUDE_DIR)/feeds.mk
endif

# invoke ipkg-build with some default options
IPKG_BUILD:= \
  $(SCRIPT_DIR)/ipkg-build -c -o 0 -g 0

IPKG_REMOVE:= \
  $(SCRIPT_DIR)/ipkg-remove

IPKG_STATE_DIR:=$(TARGET_DIR)/usr/lib/opkg

# 1: package name
# 2: variable name
# 3: variable suffix
# 4: file is a script
define BuildIPKGVariable
ifdef Package/$(1)/$(2)
  $$(IPKG_$(1)) : VAR_$(2)$(3)=$$(Package/$(1)/$(2))
  $(call shexport,Package/$(1)/$(2))
  $(1)_COMMANDS += echo "$$$$$$$$$(call shvar,Package/$(1)/$(2))" > $(2)$(3); $(if $(4),chmod 0755 $(2)$(3);)
endif
endef

PARENL :=(
PARENR :=)

dep_split=$(subst :,$(space),$(1))
dep_rem=$(subst !,,$(subst $(strip $(PARENL)),,$(subst $(strip $(PARENR)),,$(word 1,$(call dep_split,$(1))))))
dep_confvar=$(strip $(foreach cond,$(subst ||, ,$(call dep_rem,$(1))),$(CONFIG_$(cond))))
dep_pos=$(if $(call dep_confvar,$(1)),$(call dep_val,$(1)))
dep_neg=$(if $(call dep_confvar,$(1)),,$(call dep_val,$(1)))
dep_if=$(if $(findstring !,$(1)),$(call dep_neg,$(1)),$(call dep_pos,$(1)))
dep_val=$(word 2,$(call dep_split,$(1)))
strip_deps=$(strip $(subst +,,$(filter-out @%,$(1))))
filter_deps=$(foreach dep,$(call strip_deps,$(1)),$(if $(findstring :,$(dep)),$(call dep_if,$(dep)),$(dep)))

define AddDependency
  $$(if $(1),$$(if $(2),$$(foreach pkg,$(1),$$(IPKG_$$(pkg))): $$(foreach pkg,$(2),$$(IPKG_$$(pkg)))))
endef

define FixupReverseDependencies
  DEPS := $$(filter %:$(1),$$(IDEPEND))
  DEPS := $$(patsubst %:$(1),%,$$(DEPS))
  DEPS := $$(filter $$(DEPS),$$(IPKGS))
  $(call AddDependency,$$(DEPS),$(1))
endef

define FixupDependencies
  DEPS := $$(filter $(1):%,$$(IDEPEND))
  DEPS := $$(patsubst $(1):%,%,$$(DEPS))
  DEPS := $$(filter $$(DEPS),$$(IPKGS))
  $(call AddDependency,$(1),$$(DEPS))
endef

ifneq ($(PKG_NAME),toolchain)
  define CheckDependencies
	@( \
		rm -f $(PKG_INFO_DIR)/$(1).missing; \
		( \
			export \
				READELF=$(TARGET_CROSS)readelf \
				OBJCOPY=$(TARGET_CROSS)objcopy \
				XARGS="$(XARGS)"; \
			$(SCRIPT_DIR)/gen-dependencies.sh "$$(IDIR_$(1))"; \
		) | while read FILE; do \
			grep -qxF "$$$$FILE" $(PKG_INFO_DIR)/$(1).provides || \
				echo "$$$$FILE" >> $(PKG_INFO_DIR)/$(1).missing; \
		done; \
		if [ -f "$(PKG_INFO_DIR)/$(1).missing" ]; then \
			echo "Package $(1) is missing dependencies for the following libraries:" >&2; \
			cat "$(PKG_INFO_DIR)/$(1).missing" >&2; \
			false; \
		fi; \
	)
  endef
endif

_addsep=$(word 1,$(1))$(foreach w,$(wordlist 2,$(words $(1)),$(1)),$(strip $(2) $(w)))
_cleansep=$(subst $(space)$(2)$(space),$(2)$(space),$(1))
mergelist=$(call _cleansep,$(call _addsep,$(1),$(comma)),$(comma))
addfield=$(if $(strip $(2)),$(1): $(2))
_define=define
_endef=endef

ifeq ($(DUMP),)
  define BuildTarget/ipkg
    ABIV_$(1):=$(call GetABISuffix,$(1))
    PDIR_$(1):=$(call FeedPackageDir,$(1))
    IPKG_$(1):=$$(PDIR_$(1))/$(1)$$(ABIV_$(1))_$(VERSION)_$(PKGARCH).ipk
    IDIR_$(1):=$(PKG_BUILD_DIR)/ipkg-$(PKGARCH)/$(1)
    KEEP_$(1):=$(strip $(call Package/$(1)/conffiles))

    ifeq ($(BUILD_VARIANT),$$(if $$(VARIANT),$$(VARIANT),$(BUILD_VARIANT)))
    do_install=
    ifdef Package/$(1)/install
      do_install=yes
    endif
    ifdef Package/$(1)/install-overlay
      do_install=yes
    endif
    ifdef do_install
      ifneq ($(CONFIG_PACKAGE_$(1))$(DEVELOPER),)
        IPKGS += $(1)
        $(_pkg_target)compile: $$(IPKG_$(1)) $(PKG_INFO_DIR)/$(1).provides $(PKG_BUILD_DIR)/.pkgdir/$(1).installed
        prepare-package-install: $$(IPKG_$(1))
        compile: $(STAGING_DIR_ROOT)/stamp/.$(1)_installed
      else
        $(if $(CONFIG_PACKAGE_$(1)),$$(info WARNING: skipping $(1) -- package not selected))
      endif

      .PHONY: $(PKG_INSTALL_STAMP).$(1)
      ifeq ($(CONFIG_PACKAGE_$(1)),y)
        compile: $(PKG_INSTALL_STAMP).$(1)
      endif
      $(PKG_INSTALL_STAMP).$(1): prepare-package-install
		echo "$(1)" >> $(PKG_INSTALL_STAMP)
    endif
    endif

    DEPENDS:=$(call PKG_FIXUP_DEPENDS,$(1),$(DEPENDS))
    IDEPEND_$(1):=$$(call filter_deps,$$(DEPENDS))
    IDEPEND += $$(patsubst %,$(1):%,$$(IDEPEND_$(1)))
    $(FixupDependencies)
    $(FixupReverseDependencies)

    $(eval $(call BuildIPKGVariable,$(1),conffiles))
    $(eval $(call BuildIPKGVariable,$(1),preinst,,1))
    $(eval $(call BuildIPKGVariable,$(1),postinst,-pkg,1))
    $(eval $(call BuildIPKGVariable,$(1),prerm,-pkg,1))
    $(eval $(call BuildIPKGVariable,$(1),postrm,,1))

    $(PKG_BUILD_DIR)/.pkgdir/$(1).installed : export PATH=$$(TARGET_PATH_PKG)
    $(PKG_BUILD_DIR)/.pkgdir/$(1).installed: $(STAMP_BUILT)
	rm -rf $$@ $(PKG_BUILD_DIR)/.pkgdir/$(1)
	mkdir -p $(PKG_BUILD_DIR)/.pkgdir/$(1)
	$(call Package/$(1)/install,$(PKG_BUILD_DIR)/.pkgdir/$(1))
	$(call Package/$(1)/install_lib,$(PKG_BUILD_DIR)/.pkgdir/$(1))
	touch $$@

    $(STAGING_DIR_ROOT)/stamp/.$(1)_installed: $(PKG_BUILD_DIR)/.pkgdir/$(1).installed
	mkdir -p $(STAGING_DIR_ROOT)/stamp
	$(if $(ABI_VERSION),echo '$(ABI_VERSION)' | cmp -s - $(PKG_INFO_DIR)/$(1).version || echo '$(ABI_VERSION)' > $(PKG_INFO_DIR)/$(1).version)
	$(call locked,$(CP) $(PKG_BUILD_DIR)/.pkgdir/$(1)/. $(STAGING_DIR_ROOT)/,root-copy)
	touch $$@

    Package/$(1)/DEPENDS := $$(call mergelist,$$(foreach dep,$$(filter-out @%,$$(IDEPEND_$(1))),$$(dep)$$(call GetABISuffix,$$(dep))))
    ifneq ($$(EXTRA_DEPENDS),)
      Package/$(1)/DEPENDS := $$(EXTRA_DEPENDS)$$(if $$(Package/$(1)/DEPENDS),$$(comma) $$(Package/$(1)/DEPENDS))
    endif

$(_define) Package/$(1)/CONTROL
Package: $(1)$$(ABIV_$(1))
Version: $(VERSION)
$$(call addfield,Depends,$$(Package/$(1)/DEPENDS)
)$$(call addfield,Conflicts,$$(call mergelist,$(CONFLICTS))
)$$(call addfield,Provides,$$(call mergelist,$(PROVIDES)$$(if $$(ABIV_$(1)), $(1)))
)$$(call addfield,Alternatives,$$(call mergelist,$(ALTERNATIVES))
)$$(call addfield,Source,$(SOURCE)
)$$(call addfield,SourceName,$(1)
)$$(call addfield,License,$(LICENSE)
)$$(call addfield,LicenseFiles,$(LICENSE_FILES)
)$$(call addfield,Section,$(SECTION)
)$$(call addfield,Require-User,$(USERID)
)$(if $(filter hold,$(PKG_FLAGS)),Status: unknown hold not-installed
)$(if $(filter essential,$(PKG_FLAGS)),Essential: yes
)$(if $(MAINTAINER),Maintainer: $(MAINTAINER)
)Architecture: $(PKGARCH)
Installed-Size: 0
$(_endef)

    $$(IPKG_$(1)) : export CONTROL=$$(Package/$(1)/CONTROL)
    $$(IPKG_$(1)) : export DESCRIPTION=$$(Package/$(1)/description)
    $$(IPKG_$(1)) : export PATH=$$(TARGET_PATH_PKG)
    $(PKG_INFO_DIR)/$(1).provides $$(IPKG_$(1)): $(STAMP_BUILT) $(INCLUDE_DIR)/package-ipkg.mk
	@rm -rf $$(IDIR_$(1)) $$(if $$(call opkg_package_files,$(1)*),; $$(IPKG_REMOVE) $(1) $$(call opkg_package_files,$(1)*))
	mkdir -p $(PACKAGE_DIR) $$(IDIR_$(1))/CONTROL $(PKG_INFO_DIR)
	$(call Package/$(1)/install,$$(IDIR_$(1)))
	$(if $(Package/$(1)/install-overlay),mkdir -p $(PACKAGE_DIR) $$(IDIR_$(1))/rootfs-overlay)
	$(call Package/$(1)/install-overlay,$$(IDIR_$(1))/rootfs-overlay)
	-find $$(IDIR_$(1)) -name 'CVS' -o -name '.svn' -o -name '.#*' -o -name '*~'| $(XARGS) rm -rf
	@( \
		find $$(IDIR_$(1)) -name lib\*.so\* -or -name \*.ko | awk -F/ '{ print $$$$NF }'; \
		for file in $$(patsubst %,$(PKG_INFO_DIR)/%.provides,$$(IDEPEND_$(1))); do \
			if [ -f "$$$$file" ]; then \
				cat $$$$file; \
			fi; \
		done; $(Package/$(1)/extra_provides) \
	) | sort -u > $(PKG_INFO_DIR)/$(1).provides
	$(if $(PROVIDES),@for pkg in $(filter-out $(1),$(PROVIDES)); do cp $(PKG_INFO_DIR)/$(1).provides $(PKG_INFO_DIR)/$$$$pkg.provides; done)
	$(CheckDependencies)

	$(RSTRIP) $$(IDIR_$(1))
	(cd $$(IDIR_$(1))/CONTROL; \
		( \
			echo "$$$$CONTROL"; \
			printf "Description: "; echo "$$$$DESCRIPTION" | sed -e 's,^[[:space:]]*, ,g'; \
		) > control; \
		chmod 644 control; \
		( \
			echo "#!/bin/sh"; \
			echo "[ \"\$$$${IPKG_NO_SCRIPT}\" = \"1\" ] && exit 0"; \
			echo "[ -x "\$$$${IPKG_INSTROOT}/lib/functions.sh" ] || exit 0"; \
			echo ". \$$$${IPKG_INSTROOT}/lib/functions.sh"; \
			echo "default_postinst \$$$$0 \$$$$@"; \
		) > postinst; \
		( \
			echo "#!/bin/sh"; \
			echo "[ -x "\$$$${IPKG_INSTROOT}/lib/functions.sh" ] || exit 0"; \
			echo ". \$$$${IPKG_INSTROOT}/lib/functions.sh"; \
			echo "default_prerm \$$$$0 \$$$$@"; \
		) > prerm; \
		chmod 0755 postinst prerm; \
		$($(1)_COMMANDS) \
	)

    ifneq ($$(KEEP_$(1)),)
		@( \
			keepfiles=""; \
			for x in $$(KEEP_$(1)); do \
				[ -f "$$(IDIR_$(1))/$$$$x" ] || keepfiles="$$$${keepfiles:+$$$$keepfiles }$$$$x"; \
			done; \
			[ -z "$$$$keepfiles" ] || { \
				mkdir -p $$(IDIR_$(1))/lib/upgrade/keep.d; \
				for x in $$$$keepfiles; do echo $$$$x >> $$(IDIR_$(1))/lib/upgrade/keep.d/$(1); done; \
			}; \
		)
    endif

	$(INSTALL_DIR) $$(PDIR_$(1))
	$(IPKG_BUILD) $$(IDIR_$(1)) $$(PDIR_$(1))
	@[ -f $$(IPKG_$(1)) ]

    $(1)-clean:
	$$(if $$(call opkg_package_files,$(1)*),$$(IPKG_REMOVE) $(1) $$(call opkg_package_files,$(1)*))

    clean: $(1)-clean

  endef
endif
