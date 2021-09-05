include $(TOPDIR)/rules.mk

PKG_NAME:=python-docopt
PKG_VERSION:=0.6.2
PKG_RELEASE:=2

PYPI_NAME:=docopt
PKG_HASH:=49b3a825280bd66b3aa83585ef59c4a8c82f2c8a522dbe754a8bc8d08c85c491

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE-MIT

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

PYTHON3_PKG_SETUP_ARGS:=

define Package/python3-docopt
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Pythonic argument parser, that will make you smile
  URL:=http://docopt.org/
  DEPENDS:=+python3-light
endef

define Package/python3-docopt/description
  Pythonic argument parser, that will make you smile
endef

$(eval $(call Py3Package,python3-docopt))
$(eval $(call BuildPackage,python3-docopt))
$(eval $(call BuildPackage,python3-docopt-src))
