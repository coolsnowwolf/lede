#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-gdbm
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) gdbm module
  DEPENDS:=+python3-light +libgdbm
endef

$(eval $(call Py3BasePackage,python3-gdbm, \
	/usr/lib/python$(PYTHON3_VERSION)/lib-dynload/_gdbm.$(PYTHON3_SO_SUFFIX) \
))
