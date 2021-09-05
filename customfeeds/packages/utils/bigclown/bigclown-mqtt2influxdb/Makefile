# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bigclown-mqtt2influxdb
PKG_VERSION:=1.4.0
PKG_RELEASE:=1

PYPI_NAME:=mqtt2influxdb
PKG_HASH:=9fd98d2239c0b9a2482db8e55e3e5a310c5b644aa7d42c57d35ed775adb0101a

PKG_MAINTAINER:=Karel Kočí <cynerd@email.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include ../../../lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../../../lang/python/python3-package.mk

define Package/bigclown-mqtt2influxdb
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=BigClown
  TITLE:=BigCLown MQTT to Influxdb bridge
  URL:=https://github.com/bigclownlabs/bch-mqtt2influxdb
  DEPENDS:= \
    +python3-paho-mqtt \
    +python3-yaml \
    +python3-influxdb \
    +python3-jsonpath-ng \
    +python3-schema
endef

define Py3Package/bigclown-mqtt2influxdb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/init $(1)/etc/init.d/bigclown-mqtt2influxdb
	$(INSTALL_DATA) ./files/config.yml $(1)/etc/bigclown-mqtt2influxdb.yml
endef

define Package/bigclown-mqtt2influxdb/conffiles
/etc/bigclown-mqtt2influxdb.yml
endef

$(eval $(call Py3Package,bigclown-mqtt2influxdb))
$(eval $(call BuildPackage,bigclown-mqtt2influxdb))
