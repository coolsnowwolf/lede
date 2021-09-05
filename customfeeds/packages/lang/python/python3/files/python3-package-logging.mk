#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-logging
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) logging module
  DEPENDS:=+python3-light
endef

$(eval $(call Py3BasePackage,python3-logging, \
	/usr/lib/python$(PYTHON3_VERSION)/logging \
))
