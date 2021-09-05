#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=setools
PKG_VERSION:=4.4.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/SELinuxProject/setools/releases/download/$(PKG_VERSION)
PKG_HASH:=f3786677e40b7f16a226f48f233dcf835e700739614a7dbed2ff61cc9607814e
PKG_BUILD_DIR:=$(BUILD_DIR)/setools

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=Cython # Cython>=0.27

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=GPL-2.0-or-later LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING COPYING.GPL COPYING.LGPL
PKG_CPE_ID:=cpe:/a:selinuxproject:setools

include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk

define Package/setools
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+python3 +python3-pkg-resources +python3-networkx +libselinux +libsepol
  TITLE:=Policy analysis tools for SELinux
  URL:=http://selinuxproject.org/page/Main_Page
endef

define Package/setools/description
  SETools is a collection of tools and libraries designed to facilitate
  SELinux policy analysis.
endef

$(eval $(call Py3Package,setools))
$(eval $(call BuildPackage,setools))
