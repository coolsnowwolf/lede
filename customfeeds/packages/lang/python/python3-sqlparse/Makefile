include $(TOPDIR)/rules.mk

PKG_NAME:=sqlparse
PKG_VERSION:=0.4.1
PKG_RELEASE:=1

PYPI_NAME:=sqlparse
PKG_HASH:=0f91fd2e829c44362cbcfab3e9ae12e22badaa8a29ad5ff599f9ec109f0454e8

PKG_MAINTAINER:=Peter Stadler <peter.stadler@student.uibk.ac.at>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-sqlparse
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Non-validating SQL parser module.
  URL:=https://github.com/andialbrecht/sqlparse
  DEPENDS:=+python3-light
endef

define Package/python3-sqlparse/description
  A non-validating SQL parser module. It provides support for parsing, splitting and formatting SQL statements.
endef

define Py3Package/python3-sqlparse/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/$(PYTHON3_PKG_DIR)/sqlparse/__main__.py \
		$(1)/usr/bin/sqlformat
endef

$(eval $(call Py3Package,python3-sqlparse))
$(eval $(call BuildPackage,python3-sqlparse))
$(eval $(call BuildPackage,python3-sqlparse-src))
