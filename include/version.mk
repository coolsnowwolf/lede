#
# Copyright (C) 2012-2015 OpenWrt.org
# Copyright (C) 2016 LEDE Project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

# Substituted by SDK, do not remove
# REVISION:=x
# SOURCE_DATE_EPOCH:=x

PKG_CONFIG_DEPENDS += \
	CONFIG_VERSION_BUG_URL \
	CONFIG_VERSION_NUMBER \
	CONFIG_VERSION_CODE \
	CONFIG_VERSION_REPO \
	CONFIG_VERSION_DIST \
	CONFIG_VERSION_MANUFACTURER \
	CONFIG_VERSION_MANUFACTURER_URL \
	CONFIG_VERSION_PRODUCT \
	CONFIG_VERSION_SUPPORT_URL \
	CONFIG_VERSION_HWREV \

qstrip_escape=$(subst ','\'',$(call qstrip,$(1)))
#'

sanitize = $(call tolower,$(subst _,-,$(subst $(space),-,$(1))))

VERSION_NUMBER:=$(call qstrip_escape,$(CONFIG_VERSION_NUMBER))
VERSION_NUMBER:=$(if $(VERSION_NUMBER),$(VERSION_NUMBER),SNAPSHOT)

VERSION_CODE:=$(call qstrip_escape,$(CONFIG_VERSION_CODE))
VERSION_CODE:=$(if $(VERSION_CODE),$(VERSION_CODE),$(REVISION))

VERSION_REPO:=$(call qstrip_escape,$(CONFIG_VERSION_REPO))
VERSION_REPO:=$(if $(VERSION_REPO),$(VERSION_REPO),http://downloads.lede-project.org/snapshots)

VERSION_DIST:=$(call qstrip_escape,$(CONFIG_VERSION_DIST))
VERSION_DIST:=$(if $(VERSION_DIST),$(VERSION_DIST),OpenWrt)
VERSION_DIST_SANITIZED:=$(call sanitize,$(VERSION_DIST))

VERSION_MANUFACTURER:=$(call qstrip_escape,$(CONFIG_VERSION_MANUFACTURER))
VERSION_MANUFACTURER:=$(if $(VERSION_MANUFACTURER),$(VERSION_MANUFACTURER),OpenWrt)

VERSION_MANUFACTURER_URL:=$(call qstrip_escape,$(CONFIG_VERSION_MANUFACTURER_URL))
VERSION_MANUFACTURER_URL:=$(if $(VERSION_MANUFACTURER_URL),$(VERSION_MANUFACTURER_URL),http://lede-project.org/)

VERSION_BUG_URL:=$(call qstrip_escape,$(CONFIG_VERSION_BUG_URL))
VERSION_BUG_URL:=$(if $(VERSION_BUG_URL),$(VERSION_BUG_URL),http://bugs.lede-project.org/)

VERSION_SUPPORT_URL:=$(call qstrip_escape,$(CONFIG_VERSION_SUPPORT_URL))
VERSION_SUPPORT_URL:=$(if $(VERSION_SUPPORT_URL),$(VERSION_SUPPORT_URL),http://forum.lede-project.org/)

VERSION_PRODUCT:=$(call qstrip_escape,$(CONFIG_VERSION_PRODUCT))
VERSION_PRODUCT:=$(if $(VERSION_PRODUCT),$(VERSION_PRODUCT),Generic)

VERSION_HWREV:=$(call qstrip_escape,$(CONFIG_VERSION_HWREV))
VERSION_HWREV:=$(if $(VERSION_HWREV),$(VERSION_HWREV),v0)

define taint2sym
$(CONFIG_$(firstword $(subst :, ,$(subst +,,$(subst -,,$(1))))))
endef

define taint2name
$(lastword $(subst :, ,$(1)))
endef

VERSION_TAINT_SPECS := \
	-ALL_KMODS:no-all \
	-IPV6:no-ipv6 \
	+USE_GLIBC:glibc \
	+USE_MKLIBS:mklibs \
	+BUSYBOX_CUSTOM:busybox \
	+OVERRIDE_PKGS:override \

VERSION_TAINTS := $(strip $(foreach taint,$(VERSION_TAINT_SPECS), \
	$(if $(findstring +,$(taint)), \
		$(if $(call taint2sym,$(taint)),$(call taint2name,$(taint))), \
		$(if $(call taint2sym,$(taint)),,$(call taint2name,$(taint))) \
	)))

PKG_CONFIG_DEPENDS += $(foreach taint,$(VERSION_TAINT_SPECS),$(call taint2sym,$(taint)))

VERSION_SED:=$(SED) 's,%U,$(VERSION_REPO),g' \
	-e 's,%V,$(VERSION_NUMBER),g' \
	-e 's,%v,\L$(subst $(space),_,$(VERSION_NUMBER)),g' \
	-e 's,%C,$(VERSION_CODE),g' \
	-e 's,%c,\L$(subst $(space),_,$(VERSION_CODE)),g' \
	-e 's,%D,$(VERSION_DIST),g' \
	-e 's,%d,\L$(subst $(space),_,$(VERSION_DIST)),g' \
	-e 's,%R,$(REVISION),g' \
	-e 's,%T,$(BOARD),g' \
	-e 's,%S,$(BOARD)/$(if $(SUBTARGET),$(SUBTARGET),generic),g' \
	-e 's,%A,$(ARCH_PACKAGES),g' \
	-e 's,%t,$(VERSION_TAINTS),g' \
	-e 's,%M,$(VERSION_MANUFACTURER),g' \
	-e 's,%m,$(VERSION_MANUFACTURER_URL),g' \
	-e 's,%b,$(VERSION_BUG_URL),g' \
	-e 's,%s,$(VERSION_SUPPORT_URL),g' \
	-e 's,%P,$(VERSION_PRODUCT),g' \
	-e 's,%h,$(VERSION_HWREV),g'

VERSION_SED_SCRIPT:=$(subst '\'','\'\\\\\'\'',$(VERSION_SED))
