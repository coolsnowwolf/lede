#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

SRC_NAME:=libsemanage
PKG_NAME:=python3-$(SRC_NAME)
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/python-libsemanage/$(SRC_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(SRC_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/SELinuxProject/selinux/releases/download/3.2
PKG_HASH:=d722a55ca4fe2d4e2b30527720db657e6238b28079e69e2e4affeb8e733ee511
PKG_BUILD_DEPENDS:=swig/host

PKG_MAINTAINER:=Thomas Petazzoni <thomas.petazzoni@bootlin.com>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:selinuxproject:libsemanage
PYTHON3_PKG_BUILD:=0
PKG_INSTALL:=1
MAKE_PATH:=src

include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-libsemanage
  TITLE:=Python bindings for the SELinux policy management library
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  DEPENDS:=+python3 +libsemanage
  URL:=http://selinuxproject.org/page/Main_Page
endef

define Package/python3-libsemanage/description
	libsemanage is the policy management library. It uses
	libsepol for binary policy manipulation and libselinux for
	interacting with the SELinux system. It also exec's helper
	programs for loading policy and for checking whether the
	file_contexts configuration is valid (load_policy and
	setfiles from policycoreutils) presently, although this may
	change at least for the bootstrapping case (for rpm).

	This package provides the Python bindings for libsemanage.
endef #'

MAKE_FLAGS += \
	PYTHON=$(PYTHON3) \
	PYINC="-I $(PYTHON3_INC_DIR)" \
	PYLIBS="-lpython$(PYTHON3_VERSION)" \
	$(PYTHON3_VARS)

define Build/Compile
	$(call Build/Compile/Default,swigify pywrap)
endef

define Build/Install
	$(call Build/Install/Default,install-pywrap)
endef

define Build/Configure
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)$(PYTHON3_PKG_DIR)
	$(CP) $(PKG_INSTALL_DIR)$(PYTHON3_PKG_DIR)/* $(1)$(PYTHON3_PKG_DIR)
endef

define Package/python3-libsemanage/install
	$(INSTALL_DIR) $(1)$(PYTHON3_PKG_DIR)
	$(CP) $(PKG_INSTALL_DIR)$(PYTHON3_PKG_DIR)/* $(1)$(PYTHON3_PKG_DIR)
endef

$(eval $(call BuildPackage,python3-libsemanage))
