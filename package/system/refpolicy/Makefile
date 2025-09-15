#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=refpolicy
PKG_VERSION:=2.20200229
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/SELinuxProject/refpolicy/releases/download/RELEASE_2_20200229
PKG_HASH:=dec854512ed00cd057408f330c2cea4de7a4405f7a147458f59c994bf578e4b0
PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=checkpolicy/host policycoreutils/host

PKG_MAINTAINER:=Thomas Petazzoni <thomas.petazzoni@bootlin.com>
PKG_CPE_ID:=cpe:/a:tresys:refpolicy
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

TAR_OPTIONS:=--transform='s%^refpolicy%$(PKG_NAME)-$(PKG_VERSION)%' -xf -

include $(INCLUDE_DIR)/package.mk

define Package/refpolicy
  SECTION:=system
  CATEGORY:=Base system
  TITLE:=SELinux reference policy
  URL:=http://selinuxproject.org/page/Main_Page
  PKGARCH:=all
endef

define Package/refpolicy/description
	The SELinux Reference Policy project (refpolicy) is a
	complete SELinux policy that can be used as the system
	policy for a variety of systems and used as the basis for
	creating other policies. Reference Policy was originally
	based on the NSA example policy, but aims to accomplish many
	additional goals.

	The current refpolicy does not fully support OpenWRT and
	needs modifications to work with the default system file
	layout. These changes should be added as patches to the
	refpolicy that modify a single SELinux policy.

	The refpolicy works for the most part in permissive
	mode. Only the basic set of utilities are enabled in the
	example policy config and some of the pathing in the
	policies is not correct.  Individual policies would need to
	be tweaked to get everything functioning properly.
endef

# Yes, we want CC=$(HOSTCC) because the only code that checkpolicy
# builds is a small host tool that gets run as part of the build
# process.
MAKE_FLAGS += \
	SETFILES="$(STAGING_DIR_HOST)/bin/setfiles" \
	CHECKPOLICY="$(STAGING_DIR_HOSTPKG)/bin/checkpolicy" \
	CC="$(HOSTCC)" \
	CFLAGS="$(HOST_CFLAGS)"

define Build/Configure
	$(SED) "/MONOLITHIC/c\MONOLITHIC = y" $(PKG_BUILD_DIR)/build.conf
	$(SED) "/NAME/c\NAME = targeted" $(PKG_BUILD_DIR)/build.conf
	$(call Build/Compile/Default,conf)
endef

define Package/refpolicy/conffiles
/etc/selinux/config
endef

define Package/refpolicy/install
	$(INSTALL_DIR) $(1)/etc/selinux
	$(CP) $(PKG_INSTALL_DIR)/etc/selinux/* $(1)/etc/selinux/
	$(CP) ./files/selinux-config $(1)/etc/selinux/config
endef

$(eval $(call BuildPackage,refpolicy))
