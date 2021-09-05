#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=selinux-python
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/SELinuxProject/selinux/releases/download/$(PKG_VERSION)
PKG_HASH:=770855ea8120ef23007fdb9db94b1ed6e8cd77917b584ed8877bbee9c16e74fb

PKG_MAINTAINER:=Thomas Petazzoni <thomas.petazzoni@bootlin.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
PYTHON3_PKG_BUILD:=0
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk

#
# common definitions
#

define Package/selinux-python/Default
  SECTION:=utils
  DEPENDS:=+python3 +python3-libselinux
  CATEGORY:=Utilities
  URL:=http://selinuxproject.org/page/Main_Page
endef

define Package/selinux-python/Default/description
	A set of SELinux tools written in python that help with
	managing a system with SELinux enabled.
endef

MAKE_VARS = \
	PYTHON=$(HOST_PYTHON3_BIN) \
	PYTHONLIBDIR=$(PYTHON3_PKG_DIR) \
	$(PYTHON3_VARS)

define Build/Compile
	$(call Build/Compile/Default,all)
endef

#
# selinux-audit2allow
#

define Package/selinux-audit2allow
$(call Package/selinux-python/Default)
  TITLE:=selinux-audit2allow
  DEPENDS+=+python3-sepolgen +libselinux +libsepol
endef

define Package/selinux-audit2allow/description
$(call Package/selinux-python/Default/description)
  This package contains the audit2allow and audit2why tools.
endef

define Package/selinux-audit2allow/install
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR)/audit2allow DESTDIR=$(1) install
	rm -rf $(1)/usr/share/man
endef

#
# selinux-chchat
#

define Package/selinux-chcat
$(call Package/selinux-python/Default)
  TITLE:=selinux-chcat
endef

define Package/selinux-chcat/description
$(call Package/selinux-python/Default/description)
  This package contains the chcat tool.
endef

define Package/selinux-chcat/install
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR)/chcat DESTDIR=$(1) install
	rm -rf $(1)/usr/share
endef

#
# selinux-semanage
#

define Package/selinux-semanage
$(call Package/selinux-python/Default)
  TITLE:=selinux-semanage
  DEPENDS+=+python3-sepolicy +python3-libsemanage +setools
endef

define Package/selinux-semanage/description
$(call Package/selinux-python/Default/description)
  This package contains the semanage tool.
endef

define Package/selinux-semanage/install
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR)/semanage DESTDIR=$(1) install
	rm -rf $(1)/usr/share
endef

#
# python3-sepolgen
#

define Package/python3-sepolgen
$(call Package/selinux-python/Default)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python3-sepolgen
endef

define Package/python3-sepolgen/description
$(call Package/selinux-python/Default/description)
  This package contains the sepolgen Python library.
endef

define Package/python3-sepolgen/install
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR)/sepolgen DESTDIR=$(1) install
	$(INSTALL_DIR) $(1)/usr/share/sepolgen/
	$(INSTALL_DATA) $(1)/var/lib/sepolgen/perm_map $(1)/usr/share/sepolgen/perm_map
	$(RM) -rf $(1)/var
endef

#
# python3-sepolicy
#

define Package/python3-sepolicy
$(call Package/selinux-python/Default)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python3-sepolicy
endef

define Package/python3-sepolicy/description
$(call Package/selinux-python/Default/description)
  This package contains the sepolicy Python library.
endef

define Package/python3-sepolicy/install
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR)/sepolicy DESTDIR=$(1) install
	rm -rf $(1)/usr/share
endef

$(eval $(call BuildPackage,selinux-audit2allow))
$(eval $(call BuildPackage,selinux-chcat))
$(eval $(call BuildPackage,selinux-semanage))
$(eval $(call BuildPackage,python3-sepolgen))
$(eval $(call BuildPackage,python3-sepolicy))
