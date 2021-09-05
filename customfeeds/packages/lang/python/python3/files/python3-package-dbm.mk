#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-dbm
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) dbm module
  DEPENDS:=+python3-light +libdb47
endef

$(eval $(call Py3BasePackage,python3-dbm, \
	/usr/lib/python$(PYTHON3_VERSION)/dbm \
	/usr/lib/python$(PYTHON3_VERSION)/lib-dynload/_dbm.$(PYTHON3_SO_SUFFIX) \
))
