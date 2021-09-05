# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=click-log
PKG_VERSION:=0.3.2
PKG_RELEASE:=2
PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>

PKG_SOURCE_URL:=https://codeload.github.com/click-contrib/click-log/tar.gz/$(PKG_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=16babb66a2ebf22c37c1cf38753a84e6027eb8991fcf9a8487971591b8ca9812
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-click-log
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:=http://github.com/mitsuhiko/click
  TITLE:=python3-click-log
  DEPENDS:=+python3-click
endef

define Package/python3-click-log/description
  Simple and beautiful logging for click applications
endef

$(eval $(call Py3Package,python3-click-log))
$(eval $(call BuildPackage,python3-click-log))
$(eval $(call BuildPackage,python3-click-log-src))
