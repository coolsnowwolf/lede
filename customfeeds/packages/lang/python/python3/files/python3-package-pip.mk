#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-pip
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) pip module
  VERSION:=$(PYTHON3_PIP_VERSION)-$(PYTHON3_PIP_PKG_RELEASE)
  LICENSE:=MIT
  LICENSE_FILES:=LICENSE.txt
#  CPE_ID:=cpe:/a:python:pip # not currently handled this way by uscan
  DEPENDS:=+python3 +python3-setuptools +python-pip-conf
endef

define Py3Package/python3-pip/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages
	$(CP) $(PKG_BUILD_DIR)/install-pip/usr/bin/pip$(PYTHON3_VERSION) $(1)/usr/bin
	$(LN) pip$(PYTHON3_VERSION) $(1)/usr/bin/pip3
	$(LN) pip$(PYTHON3_VERSION) $(1)/usr/bin/pip
	$(CP) \
		$(PKG_BUILD_DIR)/install-pip/usr/lib/python$(PYTHON3_VERSION)/site-packages/pip \
		$(PKG_BUILD_DIR)/install-pip/usr/lib/python$(PYTHON3_VERSION)/site-packages/pip-$(PYTHON3_PIP_VERSION).dist-info \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/
	find $(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/ -path '*/__pycache__/*' -delete
	find $(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/ -type d -name __pycache__ -delete
endef

$(eval $(call Py3BasePackage,python3-pip, \
	, \
	DO_NOT_ADD_TO_PACKAGE_DEPENDS \
))
