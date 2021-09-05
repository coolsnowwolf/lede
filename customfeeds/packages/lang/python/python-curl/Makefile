#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pycurl
PKG_VERSION:=7.43.0.6
PKG_RELEASE:=1

PYPI_NAME:=$(PKG_NAME)
PKG_HASH:=8301518689daefa53726b59ded6b48f33751c383cf987b0ccfbbc4ed40281325

PKG_MAINTAINER:=Waldemar Konik <informatyk74@interia.pl>
PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING-LGPL

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../python3-package.mk

define Package/python3-curl
  CATEGORY:=Languages
  SECTION:=lang
  SUBMENU:=Python
  TITLE:=Python module interface to the cURL library
  URL:=http://pycurl.io/
  DEPENDS:=+libcurl +python3
endef

define Package/python3-curl/description
PycURL is a Python interface to libcurl, the multiprotocol file transfer library.
endef

PYTHON3_PKG_SETUP_ARGS:=

ifdef CONFIG_LIBCURL_OPENSSL
  PYTHON3_PKG_SETUP_ARGS+=--with-openssl
endif

ifdef CONFIG_LIBCURL_GNUTLS
  PYTHON3_PKG_SETUP_ARGS+=--with-gnutls
endif

ifdef CONFIG_LIBCURL_MBEDTLS
  PYTHON3_PKG_SETUP_ARGS+=--with-mbedtls
endif

ifdef CONFIG_LIBCURL_WOLFSSL
  PYTHON3_PKG_SETUP_ARGS+=--with-wolfssl
endif

$(eval $(call Py3Package,python3-curl))
$(eval $(call BuildPackage,python3-curl))
$(eval $(call BuildPackage,python3-curl-src))
