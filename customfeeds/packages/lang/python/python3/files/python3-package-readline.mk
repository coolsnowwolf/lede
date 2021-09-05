#
# Copyright (C) 2021 Alexandru Ardelean <ardeleanalex@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python3-readline
$(call Package/python3/Default)
  TITLE:=Python $(PYTHON3_VERSION) readline module
  DEPENDS:=+python3-light +libreadline +libncursesw
endef

$(eval $(call Py3BasePackage,python3-readline, \
	/usr/lib/python$(PYTHON3_VERSION)/lib-dynload/readline.$(PYTHON3_SO_SUFFIX) \
))
