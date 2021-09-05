#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-sqlite3
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) sqlite3 module
  DEPENDS:=+python3-light +libsqlite3
endef

$(eval $(call Py3BasePackage,python3-sqlite3, \
	/usr/lib/python$(PYTHON3_VERSION)/lib-dynload/_sqlite3.$(PYTHON3_SO_SUFFIX) \
	/usr/lib/python$(PYTHON3_VERSION)/sqlite3 \
))
