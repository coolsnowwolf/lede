include $(TOPDIR)/rules.mk

PKG_NAME:=colorama
PKG_VERSION:=0.4.1
PKG_RELEASE:=2

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=05eed71e2e327246ad6b38c540c4a3117230b19679b875190486ddd2d721422d

PKG_MAINTAINER:=Daniel Danzberger <daniel@dd-wrt.com>
PKG_LICENSE:=MIT

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-colorama
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=colorama
  URL:=https://github.com/tartley/colorama
  DEPENDS:=+python3
endef

define Package/python3-colorama/description
Makes ANSI escape character sequences
(for producing colored terminal text and cursor positioning) work under MS Windows.
endef

$(eval $(call Py3Package,python3-colorama))
$(eval $(call BuildPackage,python3-colorama))
$(eval $(call BuildPackage,python3-colorama-src))
