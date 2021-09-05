#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-dev
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) development files
  DEPENDS:=+python3 +python3-lib2to3
endef

define Py3Package/python3-dev/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/python$(PYTHON3_VERSION)-config $(1)/usr/bin
	$(LN) python$(PYTHON3_VERSION)-config $(1)/usr/bin/python3-config
	$(LN) python$(PYTHON3_VERSION)-config $(1)/usr/bin/python-config
	$(LN) python$(PYTHON3_VERSION)/config-$(PYTHON3_VERSION)/libpython$(PYTHON3_VERSION).a $(1)/usr/lib/
  # This depends on being called before filespec is processed
	$(SED) 's|$(TARGET_AR)|ar|g;s|$(TARGET_CROSS)readelf|readelf|g;s|$(TARGET_CC)|gcc|g;s|$(TARGET_CXX)|g++|g' \
		$(PKG_INSTALL_DIR)/usr/lib/python$(PYTHON3_VERSION)/config-$(PYTHON3_VERSION)/Makefile
endef

$(eval $(call Py3BasePackage,python3-dev, \
    /usr/lib/python$(PYTHON3_VERSION)/config-$(PYTHON3_VERSION) \
    /usr/include/python$(PYTHON3_VERSION) \
    /usr/lib/pkgconfig \
	, \
	DO_NOT_ADD_TO_PACKAGE_DEPENDS \
))
