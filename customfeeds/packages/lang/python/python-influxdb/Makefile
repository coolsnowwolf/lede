# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-influxdb
PKG_VERSION:=5.3.0
PKG_RELEASE:=1
PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>

PYPI_NAME:=influxdb
PKG_HASH:=9bcaafd57ac152b9824ab12ed19f204206ef5df8af68404770554c5b55b475f6

PKG_LICENSE:=MIT

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-influxdb
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:=https://github.com/influxdb/influxdb-python
  TITLE:=python3-influxdb
  DEPENDS:=\
    +python3-requests \
    +python3-pytz \
    +python3-six \
    +python3-dateutil \
    +python3-msgpack
endef

define Package/python3-influxdb/description
  Python client for InfluxDB
endef

$(eval $(call Py3Package,python3-influxdb))
$(eval $(call BuildPackage,python3-influxdb))
$(eval $(call BuildPackage,python3-influxdb-src))
