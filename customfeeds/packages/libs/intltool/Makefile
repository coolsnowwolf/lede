#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=intltool
PKG_VERSION:=0.51.0
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://launchpad.net/intltool/trunk/$(PKG_VERSION)/+download
PKG_HASH:=67c74d94196b153b774ab9f89b2fa6c6ba79352407037c8c14d5aeb334e959cd
PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>


HOST_BUILD_DEPENDS:=perl/host
PKG_HOST_ONLY:=1
HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

HOST_CONFIGURE_VARS+= \
        PATH="$(STAGING_DIR_HOSTPKG)/bin:$(STAGING_DIR_HOSTPKG)/usr/bin:$(PATH)"

define Package/intltool
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=set of tools to centralize translation using GNU gettext
  URL:=https://www.freedesktop.org/wiki/Software/intltool
  BUILDONLY:=1
endef

define Package/intltool/description
  intltool is a set of tools to centralize translation of many different
  file formats using GNU gettext-compatible PO files.
endef

$(eval $(call BuildPackage,intltool))
$(eval $(call HostBuild))
