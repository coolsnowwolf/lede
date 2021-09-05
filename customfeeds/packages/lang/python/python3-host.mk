#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

# Note: include this file after `include $(TOPDIR)/rules.mk in your package Makefile
#       if `python3-package.mk` is included, this will already be included

# For PYTHON3_VERSION
python3_mk_path:=$(dir $(lastword $(MAKEFILE_LIST)))
include $(python3_mk_path)python3-version.mk

HOST_PYTHON3_DIR:=$(STAGING_DIR_HOSTPKG)
HOST_PYTHON3_INC_DIR:=$(HOST_PYTHON3_DIR)/include/python$(PYTHON3_VERSION)
HOST_PYTHON3_LIB_DIR:=$(HOST_PYTHON3_DIR)/lib/python$(PYTHON3_VERSION)

HOST_PYTHON3_PKG_DIR:=$(HOST_PYTHON3_DIR)/lib/python$(PYTHON3_VERSION)/site-packages

HOST_PYTHON3_BIN:=$(HOST_PYTHON3_DIR)/bin/python$(PYTHON3_VERSION)

HOST_PYTHON3PATH:=$(HOST_PYTHON3_LIB_DIR):$(HOST_PYTHON3_PKG_DIR)

HOST_PYTHON3_VARS = \
	ARCH="$(HOST_ARCH)" \
	CC="$(HOSTCC)" \
	CCSHARED="$(HOSTCC) $(HOST_FPIC)" \
	CXX="$(HOSTCXX)" \
	LD="$(HOSTCC)" \
	LDSHARED="$(HOSTCC) -shared" \
	CFLAGS="$(HOST_CFLAGS)" \
	CPPFLAGS="$(HOST_CPPFLAGS) -I$(HOST_PYTHON3_INC_DIR)" \
	LDFLAGS="$(HOST_LDFLAGS) -lpython$(PYTHON3_VERSION) -Wl$(comma)-rpath$(comma)$(STAGING_DIR_HOSTPKG)/lib" \
	PYTHONPATH="$(HOST_PYTHON3PATH)" \
	PYTHONDONTWRITEBYTECODE=0 \
	PYTHONOPTIMIZE=""

# $(1) => directory of python script
# $(2) => python script and its arguments
# $(3) => additional variables
define HostPython3/Run
	cd "$(if $(strip $(1)),$(strip $(1)),.)" && \
	$(HOST_PYTHON3_VARS) \
	$(3) \
	$(HOST_PYTHON3_BIN) $(2)
endef

# Note: I shamelessly copied this from Yousong's logic (from python-packages);
HOST_PYTHON3_PIP:=$(STAGING_DIR_HOSTPKG)/bin/pip$(PYTHON3_VERSION)

HOST_PYTHON3_PIP_CACHE_DIR:=$(DL_DIR)/pip-cache

# Multiple concurrent pip processes can lead to errors or unexpected results: https://github.com/pypa/pip/issues/2361
# $(1) => packages to install
define HostPython3/PipInstall
	$(call locked, \
		$(HOST_PYTHON3_VARS) \
		$(HOST_PYTHON3_PIP) \
			--cache-dir "$(HOST_PYTHON3_PIP_CACHE_DIR)" \
			--disable-pip-version-check \
			install \
			--no-binary :all: \
			--require-hashes \
			$(1) \
		$(if $(CONFIG_PYTHON3_HOST_PIP_CACHE_WORLD_READABLE), \
			&& $(FIND) $(HOST_PYTHON3_PIP_CACHE_DIR) -not -type d -exec chmod go+r  '{}' \; \
			&& $(FIND) $(HOST_PYTHON3_PIP_CACHE_DIR)      -type d -exec chmod go+rx '{}' \; \
		), \
		pip \
	)
endef

# $(1) => build subdir
# $(2) => additional arguments to setup.py
# $(3) => additional variables
define HostPython3/ModSetup
	$(call HostPython3/Run, \
		$(HOST_BUILD_DIR)/$(strip $(1)), \
		setup.py $(2), \
		$(3))
endef
