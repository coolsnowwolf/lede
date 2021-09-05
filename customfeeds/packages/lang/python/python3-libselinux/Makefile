#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

SRC_NAME:=libselinux
PKG_NAME:=python3-$(SRC_NAME)
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/python-libselinux/$(SRC_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(SRC_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/SELinuxProject/selinux/releases/download/20200710
PKG_HASH:=df758ef1d9d4811051dd901ea6b029ae334ffd7c671c128beb16bce1e25ac161
PKG_BUILD_DEPENDS:=swig/host

PKG_MAINTAINER:=Thomas Petazzoni <thomas.petazzoni@bootlin.com>
PKG_LICENSE:=libselinux-1.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:selinuxproject:libselinux
PYTHON3_PKG_BUILD:=0
PKG_INSTALL:=1
MAKE_PATH:=src

include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-libselinux
  TITLE:=Python bindings for the runtime SELinux library
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  DEPENDS:=+libpcre +musl-fts +python3 +libselinux
  URL:=http://selinuxproject.org/page/Main_Page
endef

define Package/python3-libselinux/description
	libselinux is the runtime SELinux library that provides
	interfaces (e.g. library functions for the SELinux kernel
	APIs like getcon(), other support functions like
	getseuserbyname()) to SELinux-aware applications. libselinux
	may use the shared libsepol to manipulate the binary policy
	if necessary (e.g. to downgrade the policy format to an
	older version supported by the kernel) when loading policy.

	This package provides the Python bindings for libselinux.
endef

MAKE_FLAGS += \
	FTS_LDLIBS=-lfts \
	SHLIBDIR=/usr/lib \
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

define Build/InstallDev
	$(INSTALL_DIR) $(1)$(PYTHON3_PKG_DIR)
	$(CP) $(PKG_INSTALL_DIR)$(PYTHON3_PKG_DIR)/* $(1)$(PYTHON3_PKG_DIR)
endef

define Package/python3-libselinux/install
	$(INSTALL_DIR) $(1)$(PYTHON3_PKG_DIR)
	$(CP) $(PKG_INSTALL_DIR)$(PYTHON3_PKG_DIR)/* $(1)$(PYTHON3_PKG_DIR)
endef

$(eval $(call BuildPackage,python3-libselinux))
