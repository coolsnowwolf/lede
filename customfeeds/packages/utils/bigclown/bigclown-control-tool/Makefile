# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bigclown-control-tool
PKG_VERSION:=0.2.0
PKG_RELEASE:=2

PYPI_NAME:=bch
PKG_HASH:=4cd73b92757fce7275a4744baed411c867af2e671c521b90d6690b2320851d58

PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../../../lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../../../lang/python/python3-package.mk

define Package/bigclown-control-tool
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=BigClown
  TITLE:=BigCLown control tool
  URL:=https://github.com/bigclownlabs/bch-control-tool
  DEPENDS:=\
    +python3-click \
    +python3-click-log \
    +python3-paho-mqtt \
    +python3-pyserial \
    +python3-yaml \
    +python3-simplejson
endef

$(eval $(call Py3Package,bigclown-control-tool))
$(eval $(call BuildPackage,bigclown-control-tool))
