#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-rcssmin
PKG_VERSION:=1.0.6
PKG_RELEASE:=2

PKG_MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>, Eneas U de Queiroz <cote2004-github@yahoo.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PYPI_NAME:=rcssmin
PKG_HASH:=ca87b695d3d7864157773a61263e5abb96006e9ff0e021eff90cbe0e1ba18270

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-rcssmin
    SUBMENU:=Python
    SECTION:=lang
    CATEGORY:=Languages
    TITLE:=Fast CSS minifier for Python
    URL:=http://opensource.perlig.de/rcssmin/
    DEPENDS:= \
	+python3-light
endef

define Package/python3-rcssmin/description
    This module is a re-implementation aiming for speed instead of maximum compression,
    so it can be used at runtime (rather than during a preprocessing step).
    RCSSmin does syntactical compression only (removing spaces, comments and possibly
    semicolons). It does not provide semantic compression (like removing empty blocks,
    collapsing redundant properties etc).
endef

$(eval $(call Py3Package,python3-rcssmin))
$(eval $(call BuildPackage,python3-rcssmin))
$(eval $(call BuildPackage,python3-rcssmin-src))
