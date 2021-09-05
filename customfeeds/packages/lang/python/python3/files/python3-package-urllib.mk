#
# Copyright (C) 2019 Alexandru Ardelean <ardeleanalex@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-urllib
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) URL library module
  DEPENDS:=+python3-light +python3-email
endef

$(eval $(call Py3BasePackage,python3-urllib, \
	/usr/lib/python$(PYTHON3_VERSION)/urllib \
))
