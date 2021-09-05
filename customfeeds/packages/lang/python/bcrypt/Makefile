#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bcrypt
PKG_VERSION:=3.1.7
PKG_RELEASE:=4

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=0b0069c752ec14172c5f78208f1863d7ad6755a6fae6fe76ec2c80d13be41e42

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=libffi/host
HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=cffi  # cffi>=1.1

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-bcrypt
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Modern password hashing
  URL:=https://github.com/pyca/bcrypt/
  DEPENDS:= \
	+python3 \
	+python3-cffi \
	+python3-six
endef

define Package/python3-bcrypt/description
  Good password hashing for your software and your servers.
endef

$(eval $(call Py3Package,python3-bcrypt))
$(eval $(call BuildPackage,python3-bcrypt))
$(eval $(call BuildPackage,python3-bcrypt-src))
