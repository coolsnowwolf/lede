#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=checkpolicy
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/SELinuxProject/selinux/releases/download/3.2
PKG_HASH:=9b1c81fa86fe3867842164448d90c8e7ea94b2987497809c65d4caa87a5c5bc8
PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=libselinux
HOST_BUILD_DEPENDS:=libselinux/host

PKG_MAINTAINER:=Thomas Petazzoni <thomas.petazzoni@bootlin.com>
PKG_CPE_ID:=cpe:/a:selinuxproject:checkpolicy
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/checkpolicy
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=SELinux policy compiler
  URL:=http://selinuxproject.org/page/Main_Page
endef

define Package/checkpolicy/description
	checkpolicy is the SELinux policy compiler. It uses libsepol
	to generate the binary policy. checkpolicy uses the static
	libsepol since it deals with low level details of the policy
	that have not been encapsulated/abstracted by a proper
	shared library interface.
endef

include $(INCLUDE_DIR)/host-build.mk

HOST_MAKE_FLAGS += \
	PREFIX=$(STAGING_DIR_HOSTPKG)

define Package/checkpolicy/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,checkpolicy))
