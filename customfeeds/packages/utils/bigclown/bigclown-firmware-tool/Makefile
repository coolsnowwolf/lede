# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bigclown-firmware-tool
PKG_VERSION:=1.5.2
PKG_RELEASE:=1

PYPI_NAME:=bcf
PKG_HASH:=8ad897586d02433d01a58b4978516621bea388cd230640eb0b8f8f9e40f10e6c

PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../../../lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../../../lang/python/python3-package.mk

define Package/bigclown-firmware-tool
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=BigClown
  TITLE:=BigCLown firmware tool
  URL:=https://github.com/bigclownlabs/bch-firmware-tool
  DEPENDS:= \
    +python3-appdirs \
    +python3-pyserial \
    +python3-colorama \
    +python3-yaml \
    +python3-schema \
    +python3-requests \
    +python3-click \
    +python3-intelhex
endef

$(eval $(call Py3Package,bigclown-firmware-tool))
$(eval $(call BuildPackage,bigclown-firmware-tool))
