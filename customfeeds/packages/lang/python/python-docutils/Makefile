include $(TOPDIR)/rules.mk

PKG_NAME:=docutils
PKG_VERSION:=0.14
PKG_RELEASE:=2

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=51e64ef2ebfb29cae1faa133b3710143496eca21c530f3f71424d77687764274

PKG_MAINTAINER:=Daniel Danzberger <daniel@dd-wrt.com>
PKG_LICENSE:=MIT

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-docutils
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=docutils
  URL:=http://docutils.sourceforge.net
  DEPENDS:=+python3
endef

define Package/python3-docutils/description
Docutils is a modular system for processing documentation into useful formats,
such as HTML, XML, and LaTeX. For input Docutils supports reStructuredText,
an easy-to-read, what-you-see-is-what-you-get plaintext markup syntax.
endef

PYTHON3_PKG_SETUP_ARGS:=

$(eval $(call Py3Package,python3-docutils))
$(eval $(call BuildPackage,python3-docutils))
$(eval $(call BuildPackage,python3-docutils-src))
