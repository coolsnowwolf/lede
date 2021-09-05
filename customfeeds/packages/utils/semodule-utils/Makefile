#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=semodule-utils
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/SELinuxProject/selinux/releases/download/$(PKG_VERSION)
PKG_HASH:=be155654883c8dc4d05493e04a3eb9e56d28803009f1fdd721d42c68b878f757

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
HOST_BUILD_DEPENDS:=libsemanage/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/semodule-utils
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libsepol
  TITLE:=SELinux policy utilities for dealing with modules
  URL:=http://selinuxproject.org/page/Main_Page
endef

define Package/semodule-utils/description
	Semodule-utils is a collection of utilities for dealing with
	policy modules.
endef

HOST_MAKE_FLAGS += \
	PREFIX=$(STAGING_DIR_HOSTPKG) \
	SBINDIR=$(STAGING_DIR_HOSTPKG)/sbin \
	ETCDIR=$(STAGING_DIR_HOSTPKG)/etc

define Package/semodule-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,semodule-utils))
