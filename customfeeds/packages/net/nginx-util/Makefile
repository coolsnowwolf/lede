include $(TOPDIR)/rules.mk

PKG_NAME:=nginx-util
PKG_VERSION:=1.6
PKG_RELEASE:=1
PKG_MAINTAINER:=Peter Stadler <peter.stadler@student.uibk.ac.at>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= -DUBUS=y
CMAKE_OPTIONS+= -DVERSION=$(PKG_VERSION)


define Package/nginx-ssl-util/default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=Nginx configurator including SSL
  DEPENDS:=+libstdcpp +libuci +libubus +libubox +libpthread +libopenssl
  # TODO: remove after a transition period (together with below and pkg nginx):
  # It actually removes nginx-util (replacing it by a dummy pkg) to avoid
  # conflicts with nginx-ssl-util*
  DEPENDS+= +nginx-util
  EXTRA_DEPENDS:=nginx-util (>=1.4-2)
endef


define Package/nginx-ssl-util
  $(Package/nginx-ssl-util/default)
  TITLE+= (using PCRE)
  DEPENDS+= +libpcre
  CONFLICTS:=nginx-ssl-util-nopcre,
endef


define Package/nginx-ssl-util-nopcre
  $(Package/nginx-ssl-util/default)
  TITLE+= (using <regex>)
  CONFLICTS:=nginx-ssl-util
endef


define Package/nginx-ssl-util/default/description
  Utility that builds dynamically LAN listen directives for Nginx.
  Furthermore, it manages SSL directives for its server parts and can create
  corresponding (self-signed) certificates.
endef


Package/nginx-ssl-util/description = \
  $(Package/nginx-ssl-util/default/description) \
  It uses the PCRE library for performance.


Package/nginx-ssl-util-nopcre/description = \
  $(Package/nginx-ssl-util/default/description) \
  It uses the standard regex library of C++.


define Package/nginx-ssl-util/install/default
	$(INSTALL_DIR) $(1)/etc/nginx/conf.d/

	$(INSTALL_CONF) ./files/uci.conf.template $(1)/etc/nginx/
	$(LN) /var/lib/nginx/uci.conf $(1)/etc/nginx/uci.conf

	$(INSTALL_CONF) ./files/restrict_locally $(1)/etc/nginx/

	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_CONF) ./files/nginx.config $(1)/etc/config/nginx

ifneq ($(CONFIG_IPV6),y) # the used IPv6 directives have `::` in them:
	$(SED) "/::/d" $(1)/etc/nginx/restrict_locally
	$(SED) "/::/d" $(1)/etc/config/nginx
endif
endef


define Package/nginx-ssl-util/install
	$(call Package/nginx-ssl-util/install/default, $(1))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nginx-ssl-util $(1)/usr/bin/nginx-util
endef


define Package/nginx-ssl-util-nopcre/install
	$(call Package/nginx-ssl-util/install/default, $(1))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nginx-ssl-util-nopcre \
		$(1)/usr/bin/nginx-util
endef


define Package/nginx-ssl-util/prerm
#!/bin/sh

[ -n "$${IPKG_INSTROOT}" ] && exit 0
[ "$${PKG_UPGRADE}" = "1" ] && exit 0
case "$$(/sbin/uci get "nginx.global.uci_enable" 2>/dev/null)" in
	1|on|true|yes|enabled) ;;
	*) exit 0;;
esac

eval "$$(/usr/bin/nginx-util get_env)" &&
[ "$$(/sbin/uci get "nginx.$${LAN_NAME}.$${MANAGE_SSL}" 2>/dev/null)" = \
	"self-signed" ] &&
cd "/etc/nginx" &&
rm -f "$$(/sbin/uci get "nginx.$${LAN_NAME}.ssl_certificate")" \
	"$$(/sbin/uci get "nginx.$${LAN_NAME}.ssl_certificate_key")"

exit 0
endef


Package/nginx-ssl-util-nopcre/prerm = $(Package/nginx-ssl-util/prerm)


$(eval $(call BuildPackage,nginx-ssl-util))
$(eval $(call BuildPackage,nginx-ssl-util-nopcre))


# TODO: remove after a transition period (together with above and pkg nginx):
# It replaces nginx-util by a dummy pkg for a smooth upgrade of nginx*

define Package/nginx-util
  TITLE:=Dummy package for removing nginx-util when upgrading.
  DEPENDS:=+libstdcpp +libubus +libubox +libpthread
  PKGARCH:=all
endef

define Package/nginx-util/install
	$(INSTALL_DIR) $(1)/usr/bin
endef

$(eval $(call BuildPackage,nginx-util))
