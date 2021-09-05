#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fdm
PKG_VERSION:=2.0
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/nicm/fdm/releases/download/$(PKG_VERSION)
PKG_HASH:=06b28cb6b792570bc61d7e29b13d2af46b92fea77e058b2b17e11e8f7ed0cea4

PKG_MAINTAINER:=Dmitry V. Zimin <pfzim@mail.ru>
PKG_LICENSE:=BSD-2-Clause

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/fdm
  SECTION:=mail
  CATEGORY:=Mail
  TITLE:=fetch mail and deliver
  URL:=https://github.com/nicm/fdm
  MENU:=1
  DEPENDS:=+tdb +zlib +libopenssl +FDM_WITH_PCRE:libpcre
  USERID:=_fdm=99:_fdm=99
endef

define Package/fdm/description
  fdm is a simple, lightweight replacement for mail fetch, filter
  and delivery programs such as fetchmail and procmail. It can
  fetch using POP3 or IMAP (with SSL) or from stdin, and deliver
  to a pipe, file, maildir, mbox or SMTP server, based on $(if $(CONFIG_FDM_WITH_PCRE),PCRE,POSIX regexp)
endef

ifdef CONFIG_FDM_WITH_PCRE
	CONFIGURE_ARGS += --enable-pcre
endif

define Package/fdm/config
	source "$(SOURCE)/Config.in"
endef

define Package/fdm/conffiles
/etc/fdm.conf
endef

define Package/fdm/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/etc/* $(1)/etc/
	$(INSTALL_DIR) $(1)/opt/fdm
	chmod a+rwx $(1)/opt/fdm
endef

define Package/fdm/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	echo "Creating cron job template for user _fdm..."
	(crontab -l -u _fdm 2>/dev/null; echo "# */13 * * * * fdm -q fetch") | crontab -u _fdm -
	echo "Please, edit file /etc/fdm.conf and enable cron job!"
fi
exit 0
endef

define Package/fdm/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	echo "Don't forget disable cron job!"
	echo "# crontab -r -u _fdm"
fi
exit 0
endef

$(eval $(call BuildPackage,fdm))
