#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifneq ($(DUMP),)

dumpinfo: FORCE

define Config/template
Preconfig: $(1)
Preconfig-Type: $(2)
Preconfig-Default: $(3)
Preconfig-Label: $(4)

endef

define Config
  Preconfig/$(1) = $$(call Config/template,$(1),$(2),$(3),$(4))
  preconfig_$$(1) += $(1)
endef

define Dumpinfo/Package
$(info Package: $(1)
$(if $(MENU),Menu: $(MENU)
)$(if $(SUBMENU),Submenu: $(SUBMENU)
)$(if $(SUBMENUDEP),Submenu-Depends: $(SUBMENUDEP)
)$(if $(DEFAULT),Default: $(DEFAULT)
)$(if $(findstring $(PREREQ_CHECK),1),Prereq-Check: 1
)Version: $(VERSION)
Depends: $(call PKG_FIXUP_DEPENDS,$(1),$(DEPENDS))
Conflicts: $(CONFLICTS)
Menu-Depends: $(MDEPENDS)
Provides: $(PROVIDES)
$(if $(VARIANT),Build-Variant: $(VARIANT)
$(if $(DEFAULT_VARIANT),Default-Variant: $(VARIANT)
))$(if $(PKG_BUILD_DEPENDS),Build-Depends: $(PKG_BUILD_DEPENDS)
)$(if $(HOST_BUILD_DEPENDS),Build-Depends/host: $(HOST_BUILD_DEPENDS)
)$(if $(BUILD_TYPES),Build-Types: $(BUILD_TYPES)
)Section: $(SECTION)
Category: $(CATEGORY)
$(if $(filter nonshared,$(PKGFLAGS)),,Repository: $(if $(FEED),$(FEED),base)
)Title: $(TITLE)
Maintainer: $(MAINTAINER)
$(if $(USERID),Require-User: $(USERID)
)Source: $(PKG_SOURCE)
$(if $(PKG_LICENSE),License: $(PKG_LICENSE)
)$(if $(PKG_LICENSE_FILES),LicenseFiles: $(PKG_LICENSE_FILES)
)Type: $(if $(Package/$(1)/targets),$(Package/$(1)/targets),$(if $(PKG_TARGETS),$(PKG_TARGETS),ipkg))
$(if $(KCONFIG),Kernel-Config: $(KCONFIG)
)$(if $(BUILDONLY),Build-Only: $(BUILDONLY)
)$(if $(HIDDEN),Hidden: $(HIDDEN)
)Description: $(if $(Package/$(1)/description),$(Package/$(1)/description),$(TITLE))
$(if $(URL),$(URL)
)$(MAINTAINER)
@@
$(if $(Package/$(1)/config),Config:
$(Package/$(1)/config)
@@
)$(foreach pc,$(preconfig_$(1)),
$(Preconfig/$(pc))))
endef

define Feature/Default
  TARGET_NAME:=
  TARGET_TITLE:=
  PRIORITY:=
  NAME:=
endef

define Feature
  $(eval $(Feature/Default))
  $(eval $(Feature/$(1)))
  $(if $(DUMP),$(call Dumpinfo/Feature,$(1)))
endef

define Dumpinfo/Feature
$(info Feature: $(TARGET_NAME)_$(1)
Target-Name: $(TARGET_NAME)
Target-Title: $(TARGET_TITLE)
Feature-Name: $(NAME)
$(if $(PRIORITY),Feature-Priority: $(PRIORITY)
)Feature-Description:
$(Feature/$(1)/description)
@@
)
endef

endif
