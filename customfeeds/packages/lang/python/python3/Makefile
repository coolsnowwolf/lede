#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

# The file included below defines PYTHON3_VERSION
include ../python3-version.mk

PKG_NAME:=python3
PKG_RELEASE:=2
PKG_VERSION:=$(PYTHON3_VERSION).$(PYTHON3_VERSION_MICRO)

PKG_SOURCE:=Python-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.python.org/ftp/python/$(PKG_VERSION)
PKG_HASH:=0c5a140665436ec3dbfbb79e2dfb6d192655f26ef4a29aeffcb6d1820d716d83

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>, Jeffery To <jeffery.to@gmail.com>
PKG_LICENSE:=Python/2.0
PKG_LICENSE_FILES:=LICENSE Doc/copyright.rst Doc/license.rst Modules/_ctypes/darwin/LICENSE Modules/_ctypes/libffi_osx/LICENSE Modules/expat/COPYING
PKG_CPE_ID:=cpe:/a:python:python

# This file provides the necsessary host build variables
include ../python3-host.mk

# For Py3Package
PYTHON3_PKG_BUILD:=0
include ../python3-package.mk

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
HOST_BUILD_PARALLEL:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/Python-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/Python-$(PKG_VERSION)

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_python3-pkg-resources \
	CONFIG_PACKAGE_python3-setuptools CONFIG_PACKAGE_python3-pip \
	CONFIG_PYTHON3_BLUETOOTH_SUPPORT

PKG_BUILD_DEPENDS:=python3/host
HOST_BUILD_DEPENDS:=bzip2/host expat/host libffi/host

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/python3/Default
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Python $(PYTHON3_VERSION) programming language
  URL:=https://www.python.org/
endef

define Package/python3/Default/description
 Python is a dynamic object-oriented programming language that can be used
 for many kinds of software development. It offers strong support for
 integration with other languages and tools, comes with extensive standard
 libraries, and can be learned in a few days. Many Python programmers
 report substantial productivity gains and feel the language encourages
 the development of higher quality, more maintainable code.
endef

define Package/libpython3
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) core library
  DEPENDS:=+libpthread +zlib
  ABI_VERSION:=$(PYTHON3_VERSION)
endef

define Package/libpython3/description
  This package contains only core Python library.
endef

define Package/python3-base
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) interpreter
  DEPENDS:=+libpthread +zlib +libpython3
endef

define Package/python3-base/description
  This package contains only the interpreter and the bare minimum
  for the interpreter to start.
endef

define Package/python3-light
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) light installation
  DEPENDS:=+python3-base +libffi +libbz2 +PYTHON3_BLUETOOTH_SUPPORT:bluez-libs +libuuid
endef

define Package/python3-light/config
  source "$(SOURCE)/Config-python3-light.in"
endef

define Package/python3-light/description
  This package is essentially the python3-base package plus
  a few of the rarely used (and big) libraries stripped out
  into separate packages.
endef

PYTHON3_LIB_FILES_DEL:=
PYTHON3_PACKAGES:=
PYTHON3_SO_SUFFIX:=cpython-$(PYTHON3_VERSION_MAJOR)$(PYTHON3_VERSION_MINOR).so
PYTHON3_PACKAGES_DEPENDS:=
define Py3BasePackage
  PYTHON3_PACKAGES+=$(1)
  ifeq ($(3),)
    PYTHON3_PACKAGES_DEPENDS+=$(1)
  endif
  PYTHON3_LIB_FILES_DEL+=$(2)
  define Py3Package/$(1)/filespec
    ifneq ($(2),)
      $(subst $(space),$(newline),$(foreach lib_file,$(2),+|$(lib_file)))
      -|/usr/lib/python$(PYTHON3_VERSION)/*/test
      -|/usr/lib/python$(PYTHON3_VERSION)/*/tests
    endif
  endef
  Py3Package/$(1)/install?=:
endef

include ./files/python3-package-*.mk

define Package/python3
$(call Package/python3/Default)
  DEPENDS:=+python3-light $(foreach package,$(PYTHON3_PACKAGES_DEPENDS),+$(package))
endef

define Package/python3/description
  This package contains the (almost) full Python install.
  It's python3-light + all other packages.
endef

TARGET_CONFIGURE_OPTS+= \
	READELF="$(TARGET_CROSS)readelf"

MAKE_FLAGS+=\
	LD="$(TARGET_CC)"

EXTRA_CFLAGS+= \
	-DNDEBUG -fno-inline
EXTRA_LDFLAGS+= \
	-L$(PKG_BUILD_DIR) \
	-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib

DISABLE_BLUETOOTH:= \
	ac_cv_header_bluetooth_bluetooth_h=no \
	ac_cv_header_bluetooth_h=no

CONFIGURE_ARGS+= \
	--enable-optimizations \
	--enable-shared \
	--with-system-ffi \
	--without-cxx-main \
	--without-ensurepip \
	--without-pymalloc \
	$(if $(CONFIG_IPV6),--enable-ipv6) \
	$(if $(findstring mips,$(CONFIG_ARCH)),,--with-lto) \
	CONFIG_SITE="$(PKG_BUILD_DIR)/config.site" \
	OPT="$(TARGET_CFLAGS)" \
	$(if $(CONFIG_PYTHON3_BLUETOOTH_SUPPORT),,$(DISABLE_BLUETOOTH))

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) ./files/config.site $(PKG_BUILD_DIR)/config.site
endef

ifdef CONFIG_PACKAGE_python3-setuptools
PYTHON3_SETUPTOOLS_BUILD:=1
endif

ifdef CONFIG_PACKAGE_python3-pkg-resources
PYTHON3_SETUPTOOLS_BUILD:=1
endif

ifeq ($(PYTHON3_SETUPTOOLS_BUILD),1)
define Build/Compile/python3-setuptools
	$(HOST_PYTHON3_PIP) \
		--disable-pip-version-check \
		--cache-dir "$(DL_DIR)/pip-cache" \
		install \
		--ignore-installed \
		--root=$(PKG_BUILD_DIR)/install-setuptools \
		--prefix=/usr \
		$(PKG_BUILD_DIR)/Lib/ensurepip/_bundled/setuptools-$(PYTHON3_SETUPTOOLS_VERSION)-py3-none-any.whl
	$(call PatchDir,$(PKG_BUILD_DIR)/install-setuptools/usr/lib/python$(PYTHON3_VERSION)/site-packages,./patches-setuptools,)
endef
endif # CONFIG_PACKAGE_python3-setuptools

ifdef CONFIG_PACKAGE_python3-pip
define Build/Compile/python3-pip
	$(HOST_PYTHON3_PIP) \
		--disable-pip-version-check \
		--cache-dir "$(DL_DIR)/pip-cache" \
		install \
		--ignore-installed \
		--root=$(PKG_BUILD_DIR)/install-pip \
		--prefix=/usr \
		$(PKG_BUILD_DIR)/Lib/ensurepip/_bundled/pip-$(PYTHON3_PIP_VERSION)-py3-none-any.whl
	$(call PatchDir,$(PKG_BUILD_DIR)/install-pip/usr/lib/python$(PYTHON3_VERSION)/site-packages,./patches-pip,)
endef
endif # CONFIG_PACKAGE_python3-pip

define Build/Compile
	$(call Build/Compile/Default)
	# Use host pip to install python-setuptools
	$(call Build/Compile/python3-setuptools)
	$(call Build/Compile/python3-pip)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(2)/bin
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/python$(PYTHON3_VERSION) \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/python$(PYTHON3_VERSION) \
		$(PKG_INSTALL_DIR)/usr/lib/libpython$(PYTHON3_VERSION).so* \
		$(1)/usr/lib/
	grep \
		'^_PYTHON_HOST_PLATFORM=' \
		$(PKG_INSTALL_DIR)/usr/lib/python$(PYTHON3_VERSION)/config-$(PYTHON3_VERSION)/Makefile > \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/config-$(PYTHON3_VERSION)/Makefile-vars
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/python*.pc \
		$(1)/usr/lib/pkgconfig
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/python$(PYTHON3_VERSION)-config \
		$(2)/bin/
	$(SED) \
		's|^prefix_real=.*$$$$|prefix_real="$(PYTHON3_DIR)"|' \
		$(2)/bin/python$(PYTHON3_VERSION)-config
endef

PYTHON3_BASE_LIB_FILES:= \
	/usr/lib/python$(PYTHON3_VERSION)/encodings \
	/usr/lib/python$(PYTHON3_VERSION)/_collections_abc.py \
	/usr/lib/python$(PYTHON3_VERSION)/_sitebuiltins.py \
	/usr/lib/python$(PYTHON3_VERSION)/_sysconfigdata.py \
	/usr/lib/python$(PYTHON3_VERSION)/_weakrefset.py \
	/usr/lib/python$(PYTHON3_VERSION)/abc.py \
	/usr/lib/python$(PYTHON3_VERSION)/codecs.py \
	/usr/lib/python$(PYTHON3_VERSION)/genericpath.py \
	/usr/lib/python$(PYTHON3_VERSION)/io.py \
	/usr/lib/python$(PYTHON3_VERSION)/os.py \
	/usr/lib/python$(PYTHON3_VERSION)/posixpath.py \
	/usr/lib/python$(PYTHON3_VERSION)/site.py \
	/usr/lib/python$(PYTHON3_VERSION)/sysconfig.py \
	/usr/lib/python$(PYTHON3_VERSION)/stat.py

PYTHON3_LIB_FILES_DEL+=$(PYTHON3_BASE_LIB_FILES)

define Py3Package/python3-base/filespec
+|/usr/bin/python$(PYTHON3_VERSION)
$(subst $(space),$(newline),$(foreach lib_file,$(PYTHON3_BASE_LIB_FILES),+|$(lib_file)))
endef

define Py3Package/python3-light/filespec
+|/usr/lib/python$(PYTHON3_VERSION)
-|/usr/lib/python$(PYTHON3_VERSION)/distutils/cygwinccompiler.py
-|/usr/lib/python$(PYTHON3_VERSION)/distutils/command/wininst*
-|/usr/lib/python$(PYTHON3_VERSION)/ensurepip
-|/usr/lib/python$(PYTHON3_VERSION)/idlelib
-|/usr/lib/python$(PYTHON3_VERSION)/tkinter
-|/usr/lib/python$(PYTHON3_VERSION)/turtledemo
-|/usr/lib/python$(PYTHON3_VERSION)/lib-dynload/_test*.so
-|/usr/lib/python$(PYTHON3_VERSION)/pdb.doc
-|/usr/lib/python$(PYTHON3_VERSION)/test
-|/usr/lib/python$(PYTHON3_VERSION)/webbrowser.py
-|/usr/lib/python$(PYTHON3_VERSION)/*/test
-|/usr/lib/python$(PYTHON3_VERSION)/*/tests
-|/usr/lib/python$(PYTHON3_VERSION)/_osx_support.py
$(subst $(space),$(newline),$(foreach lib_file,$(PYTHON3_LIB_FILES_DEL),-|$(lib_file)))
endef

define Package/libpython3/install
	# Adding the lib-dynload folder (even just empty) suppresses 2 warnings when starting Python
	$(INSTALL_DIR) $(1)/usr/lib/python$(PYTHON3_VERSION)/lib-dynload/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpython$(PYTHON3_VERSION).so* $(1)/usr/lib/
endef

define Py3Package/python3-base/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) python$(PYTHON3_VERSION) $(1)/usr/bin/python3
	$(LN) python$(PYTHON3_VERSION) $(1)/usr/bin/python
  # This depends on being called before filespec is processed
	$(SED) 's|$(TARGET_AR)|ar|g;s|$(TARGET_CROSS)readelf|readelf|g;s|$(TARGET_CC)|gcc|g;s|$(TARGET_CXX)|g++|g' \
		$(PKG_INSTALL_DIR)/usr/lib/python$(PYTHON3_VERSION)/_sysconfigdata.py
endef

Py3Package/python3-light/install:=:
Py3Package/python3/install:=:

define Py3Package/python3/filespec
-|$(PYTHON3_PKG_DIR)
endef

HOST_LDFLAGS += \
	-Wl$(comma)-rpath$(comma)$(STAGING_DIR_HOSTPKG)/lib

ifeq ($(HOST_OS),Linux)
HOST_LDFLAGS += \
	-Wl,--no-as-needed -lrt
endif

ifeq ($(HOST_OS),Darwin)
HOST_CONFIGURE_VARS += \
	ac_cv_header_libintl_h=no
HOST_MAKE_VARS += \
	USE_PYTHON_CONFIG_PY=1
endif

HOST_CONFIGURE_ARGS+= \
	--enable-optimizations \
	--with-ensurepip=upgrade \
	--with-system-expat=$(STAGING_DIR_HOSTPKG) \
	--without-cxx-main \
	--without-pymalloc \
	CONFIG_SITE=

define Host/Configure
	$(SED) 's/^ENABLE_USER_SITE = None$$$$/ENABLE_USER_SITE = False/' $(HOST_BUILD_DIR)/Lib/site.py
	$(call Host/Configure/Default)
endef

define Host/Compile
	$(call Host/Compile/Default,python)
	$(call Host/Compile/Default,sharedmods)
endef

define Host/Install
	$(if $(wildcard $(HOST_PYTHON3_PKG_DIR)/.setuptools_installed_$(PYTHON3_SETUPTOOLS_VERSION)-$(PYTHON3_SETUPTOOLS_PKG_RELEASE)),,
		rm -rf \
			$(HOST_PYTHON3_PKG_DIR)/pkg_resources \
			$(HOST_PYTHON3_PKG_DIR)/setuptools \
			$(HOST_PYTHON3_PKG_DIR)/setuptools-* \
			$(HOST_PYTHON3_PKG_DIR)/.setuptools-patched* \
			$(HOST_PYTHON3_PKG_DIR)/.setuptools_installed_*
	)
	$(if $(wildcard $(HOST_PYTHON3_PKG_DIR)/.pip_installed_$(PYTHON3_PIP_VERSION)-$(PYTHON3_PIP_PKG_RELEASE)),,
		rm -rf \
			$(HOST_PYTHON3_PKG_DIR)/pip \
			$(HOST_PYTHON3_PKG_DIR)/pip-* \
			$(HOST_PYTHON3_PKG_DIR)/.pip-patched* \
			$(HOST_PYTHON3_PKG_DIR)/.pip_installed_*
	)
	$(call Host/Install/Default)
	$(if $(wildcard $(HOST_PYTHON3_PKG_DIR)/.setuptools_installed_$(PYTHON3_SETUPTOOLS_VERSION)-$(PYTHON3_SETUPTOOLS_PKG_RELEASE)),,
		$(call HostPatchDir,$(HOST_PYTHON3_PKG_DIR),./patches-setuptools,)
		touch $(HOST_PYTHON3_PKG_DIR)/.setuptools_installed_$(PYTHON3_SETUPTOOLS_VERSION)-$(PYTHON3_SETUPTOOLS_PKG_RELEASE)
	)
	$(if $(wildcard $(HOST_PYTHON3_PKG_DIR)/.pip_installed_$(PYTHON3_PIP_VERSION)-$(PYTHON3_PIP_PKG_RELEASE)),,
		$(call HostPatchDir,$(HOST_PYTHON3_PKG_DIR),./patches-pip,)
		touch $(HOST_PYTHON3_PKG_DIR)/.pip_installed_$(PYTHON3_PIP_VERSION)-$(PYTHON3_PIP_PKG_RELEASE)
	)
endef

$(eval $(call HostBuild))

$(foreach package, $(PYTHON3_PACKAGES),  \
	$(eval $(call Py3Package,$(package))) \
	$(eval $(call BuildPackage,$(package))) \
	$(eval $(call BuildPackage,$(package)-src)) \
)

$(eval $(call BuildPackage,libpython3))

$(eval $(call Py3Package,python3-base))
$(eval $(call Py3Package,python3-light))
$(eval $(call Py3Package,python3))

$(eval $(call BuildPackage,python3-base))
$(eval $(call BuildPackage,python3-light))
$(eval $(call BuildPackage,python3))

$(eval $(call BuildPackage,python3-base-src))
$(eval $(call BuildPackage,python3-light-src))
