#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=policycoreutils
PKG_VERSION:=3.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/SELinuxProject/selinux/releases/download/$(PKG_VERSION)
PKG_HASH:=4199040ced8a81f2ddd0522b4faf2aba62fc821473f4051dc8474fb1c4a01078
PKG_INSTALL:=1
HOST_BUILD_DEPENDS:=libsemanage/host gettext-full/host
PKG_BUILD_DEPENDS:=BUSYBOX_CONFIG_PAM:libpam gettext-full/host

PKG_MAINTAINER:=Thomas Petazzoni <thomas.petazzoni@bootlin.com>
PKG_CPE_ID:=cpe:/a:selinuxproject:policycoreutils
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/host-build.mk

DIR_USR_BIN:= \
	newrole \
	secon \
	sestatus

DIR_USR_SBIN:= \
	load_policy \
	setsebool

LIBEXEC_UTILS := \
	pp

SBIN_UTILS:= \
	restorecon_xattr \
	setfiles

USR_BIN_UTILS:= \
	newrole \
	secon \
	sestatus

USR_SBIN_UTILS:= \
	fixfiles \
	genhomedircon \
	open_init_pty \
	run_init \
	semodule \
	load_policy \
	setsebool

TARGET_LDFLAGS += $(INTL_LDFLAGS) $(if $(INTL_FULL),-lintl)

MAKE_FLAGS += \
	PAMH=$(CONFIG_BUSYBOX_CONFIG_PAM)

HOST_MAKE_FLAGS += \
	PAMH=$(CONFIG_BUSYBOX_CONFIG_PAM) \
	DESTDIR=$(STAGING_DIR_HOST) \
	PREFIX= \
	SBINDIR=/bin

HOST_LDFLAGS += -Wl,-rpath=$(STAGING_DIR_HOSTPKG)/lib

$(eval $(foreach a,$(DIR_SBIN),ALTS_$(a):=300:/sbin/$(a):/sbin/policycoreutils-$(a)$(newline)))
$(eval $(foreach a,$(DIR_USR_BIN),ALTS_$(a):=300:/usr/bin/$(a):/usr/bin/policycoreutils-$(a)$(newline)))
$(eval $(foreach a,$(DIR_USR_SBIN),ALTS_$(a):=300:/usr/sbin/$(a):/usr/sbin/policycoreutils-$(a)$(newline)))
ALTS_setfiles:=300:/sbin/restorecon:/sbin/policycoreutils-setfiles 300:/sbin/setfiles:/sbin/policycoreutils-setfiles

DEPENDS_genhomedircon:=+libsemanage $(INTL_DEPENDS)
DEPENDS_load_policy:=+libselinux $(INTL_DEPENDS)
DEPENDS_newrole:=+libselinux +libaudit +BUSYBOX_CONFIG_PAM:libpam $(INTL_DEPENDS)
DEPENDS_open_init_pty:=$(INTL_DEPENDS)
DEPENDS_pp:=+libsepol $(INTL_DEPENDS)
DEPENDS_restorecon_xattr:=+libselinux +libsepol +libaudit $(INTL_DEPENDS)
DEPENDS_run_init:=+libselinux +libaudit +BUSYBOX_CONFIG_PAM:libpam $(INTL_DEPENDS)
DEPENDS_secon:=+libselinux $(INTL_DEPENDS)
DEPENDS_semanage:=+libsemanage
DEPENDS_semodule:=+libsemanage $(INTL_DEPENDS)
DEPENDS_sestatus:=+libselinux $(INTL_DEPENDS)
DEPENDS_setfiles:=+libselinux +libsepol +libaudit $(INTL_DEPENDS)
DEPENDS_setsebool:=+libsemanage $(INTL_DEPENDS)

define Package/policycoreutils/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=SELinux policy utility
  URL:=http://selinuxproject.org/page/Main_Page
endef

define Package/policycoreutils
  $(call Package/policycoreutils/Default)
  MENU:=1
  TITLE+= common files
endef

define GenUtilPkg
  define Package/$(1)
    $(call Package/policycoreutils/Default)
    DEPENDS+= policycoreutils $(DEPENDS_$(2))
    TITLE+= $(2)
    ALTERNATIVES:=$(ALTS_$(2))
  endef

  define Package/$(1)/description
Policycoreutils is a collection of policy utilities
(originally the "core" set of utilities needed to use
SELinux, although it has grown a bit over time).

This package provides the $(2) utility.
  endef
endef

$(foreach a,$(LIBEXEC_UTILS) $(SBIN_UTILS) $(USR_BIN_UTILS) $(USR_SBIN_UTILS),$(eval $(call GenUtilPkg,policycoreutils-$(a),$(a))))

define Package/policycoreutils/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/sestatus.conf $(1)/etc
ifdef CONFIG_BUSYBOX_CONFIG_PAM
	$(INSTALL_DIR) $(1)/etc/pam.d
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/pam.d/run_init $(1)/etc/pam.d
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/pam.d/newrole $(1)/etc/pam.d
endif
endef

define BuildUtil
  define Package/$(1)/install
	$(INSTALL_DIR) $$(1)$(2)
	$(INSTALL_BIN) $$(PKG_INSTALL_DIR)$(2)/$(3) $$(1)$(2)/$(if $(ALTS_$(3)),policycoreutils-$(3),$(3))
  endef

  $$(eval $$(call BuildPackage,$(1)))
endef

$(eval $(call BuildPackage,policycoreutils))
$(foreach a,$(SBIN_UTILS),$(eval $(call BuildUtil,policycoreutils-$(a),/sbin,$(a))))
$(foreach a,$(USR_BIN_UTILS),$(eval $(call BuildUtil,policycoreutils-$(a),/usr/bin,$(a))))
$(foreach a,$(USR_SBIN_UTILS),$(eval $(call BuildUtil,policycoreutils-$(a),/usr/sbin,$(a))))
$(foreach a,$(LIBEXEC_UTILS),$(eval $(call BuildUtil,policycoreutils-$(a),/usr/libexec/selinux/hll,$(a))))
$(eval $(call HostBuild))
