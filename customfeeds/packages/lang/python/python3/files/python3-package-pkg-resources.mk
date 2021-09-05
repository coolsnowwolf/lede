#
# Copyright (C) 2019 Alexandru Ardelean <ardeleanalex@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-pkg-resources
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) pkg_resources module (part of setuptools)
  VERSION:=$(PYTHON3_SETUPTOOLS_VERSION)-$(PYTHON3_SETUPTOOLS_PKG_RELEASE)
  LICENSE:=MIT
  LICENSE_FILES:=LICENSE
#  CPE_ID:=cpe:/a:python:setuptools # not currently handled this way by uscan
  DEPENDS:=+python3
endef

define Py3Package/python3-pkg-resources/install
	$(INSTALL_DIR) $(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages
	$(CP) \
		$(PKG_BUILD_DIR)/install-setuptools/usr/lib/python$(PYTHON3_VERSION)/site-packages/pkg_resources \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages
	find $(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/ -path '*/__pycache__/*' -delete
	find $(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/ -type d -name __pycache__ -delete
endef

$(eval $(call Py3BasePackage,python3-pkg-resources, \
	, \
	DO_NOT_ADD_TO_PACKAGE_DEPENDS \
))
