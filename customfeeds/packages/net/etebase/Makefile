include $(TOPDIR)/rules.mk

PKG_NAME:=etebase
PKG_VERSION:=0.6.1
PKG_RELEASE:=1

PKG_SOURCE:=etebase-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/etesync/server/archive/v$(PKG_VERSION)
PKG_HASH:=4832c35fa1b46936bfde894a8c888989e9a37dd64ca588df22524825f3e568ec

PKG_LICENSE:=AGPL-3.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Peter Stadler <peter.stadler@student.uibk.ac.at>

PKG_BUILD_PARALLEL:=1
PYTHON3_PKG_BUILD:=0

PKG_UNPACK=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xzf $(DL_DIR)/$(PKG_SOURCE)

include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk


define Package/etebase
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	TITLE:=End-to-end encrypted backend
	URL:=https://www.etebase.com/
	DEPENDS:=+nginx-ssl +uwsgi +uwsgi-syslog-plugin +uwsgi-python3-plugin +python3-light \
		+python3-asgiref +python3-cffi +python3-django-cors-headers +python3-django \
		+python3-django-restframework +python3-drf-nested-routers +python3-msgpack \
		+python3-pycparser +python3-pynacl +python3-pytz +python3-six +python3-sqlparse
		# +psycopg2-binary would be needed for using postgres db.
	USERID:=etebase=44312
endef


define Package/etebase/description
	End-to-end encrypted backend
endef


Build/Compile:=:


define Py3Package/etebase/install
	# OpenWrt specific:
	$(INSTALL_DIR) $(1)/etc/uci-defaults/
	$(CP) ./files/81_setup-etebase $(1)/etc/uci-defaults/

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/uwsgi.init $(1)/etc/init.d/etebase

	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_CONF) ./files/uci.cfg $(1)/etc/config/etebase

	# Server configuration:
	$(INSTALL_DIR) $(1)/etc/nginx/conf.d/
	$(INSTALL_CONF) ./files/etebase.locations $(1)/etc/nginx/conf.d/

	$(INSTALL_DIR) $(1)/etc/uwsgi/vassals/
	$(INSTALL_CONF) ./files/uwsgi.ini $(1)/etc/uwsgi/vassals/etebase.available
	#init links etebase.available /var/etc/etebase/uwsgi.ini:
	$(LN) /var/etc/etebase/uwsgi.ini $(1)/etc/uwsgi/vassals/etebase.ini

	# Upstream application:
	$(INSTALL_DIR) $(1)/usr/share/etebase/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/manage.py $(1)/usr/share/etebase/
	#init creates /var/etc/etebase/server.ini from uci.cfg:
	$(LN) /var/etc/etebase/server.ini $(1)/usr/share/etebase/etebase-server.ini

	$(INSTALL_DIR) $(1)/usr/share/etebase/django_etebase/
	$(CP) $(PKG_BUILD_DIR)/django_etebase/* $(1)/usr/share/etebase/django_etebase/

	$(INSTALL_DIR) $(1)/usr/share/etebase/etebase_server/
	$(CP) $(PKG_BUILD_DIR)/etebase_server/* $(1)/usr/share/etebase/etebase_server/

	$(INSTALL_DIR) $(1)/usr/share/etebase/myauth/
	$(CP) $(PKG_BUILD_DIR)/myauth/* $(1)/usr/share/etebase/myauth/

	$(INSTALL_DIR) $(1)/usr/share/etebase/templates/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/templates/* $(1)/usr/share/etebase/templates/

	$(INSTALL_DIR) $(1)/www/etebase/static/
endef


Py3Package/etebase/filespec:=


define Package/etebase/postrm
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] && exit 0
cd /usr/share/etebase/ && rmdir */*/*/*/* */*/*/* */*/* */* * 2>/dev/null
[ "$${PKG_UPGRADE}" = "1" ] && exit 0
rm -r /www/etebase/static
rmdir --ignore-fail-on-non-empty /www/etebase /var/etc/etebase
exit 0
endef


define Package/etebase/conffiles
/etc/config/etebase
/etc/nginx/conf.d/etebase.locations
/etc/uwsgi/vassals/etebase.available
endef


$(eval $(call Py3Package,etebase))
$(eval $(call BuildPackage,etebase))
$(eval $(call BuildPackage,etebase-src))
