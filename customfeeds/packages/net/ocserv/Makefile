#
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ocserv
PKG_VERSION:=1.1.3
PKG_RELEASE:=1
PKG_USE_MIPS16:=0

PKG_BUILD_DIR :=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=ftp://ftp.infradead.org/pub/ocserv/
PKG_HASH:=1ab70c6e6ea36b613e8e171fc03b6081c4312a45ee52cc2959c068c27324107e

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
#PKG_FIXUP:=autoreconf

PKG_CONFIG_DEPENDS:= \
	CONFIG_OCSERV_PAM \
	CONFIG_OCSERV_SECCOMP \
	CONFIG_OCSERV_PROTOBUF \

include $(INCLUDE_DIR)/package.mk

define Package/ocserv/config
	source "$(SOURCE)/Config.in"
endef

define Package/ocserv
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  TITLE:=OpenConnect VPN server
  URL:=http://www.infradead.org/ocserv/
  MAINTAINER:=Nikos Mavrogiannopoulos <n.mavrogiannopoulos@gmail.com>
  DEPENDS:= +OCSERV_RADIUS:libradcli +OCSERV_HTTP_PARSER:libhttp-parser +OCSERV_SECCOMP:libseccomp +libgnutls +certtool +libncurses +libreadline +OCSERV_PAM:libpam +OCSERV_PROTOBUF:libprotobuf-c +libev +kmod-tun
  USERID:=ocserv=72:ocserv=72
endef

define Package/ocserv/description
 OpenConnect server (ocserv) is an SSL VPN server. Its purpose is to be
 a secure, small, fast and configurable VPN server. It implements the
 OpenConnect SSL VPN protocol, and has also (currently experimental)
 compatibility with clients using the AnyConnect SSL VPN protocol. The
 OpenConnect VPN protocol uses the standard IETF security protocols such
 as TLS 1.2, and Datagram TLS to provide the secure VPN service.
endef

EXTRA_CPPFLAGS+=-I$(STAGING_DIR)/usr/include/readline/
EXTRA_LDFLAGS+=-lncurses

CONFIGURE_VARS += \
	ac_cv_file__proc_self_exe=yes

CONFIGURE_ARGS+= \
	--with-pager="" \
	--with-libreadline-prefix="$(STAGING_DIR)/" \
	--without-libnl \
	--without-gssapi \
	--with-libcrypt-prefix="$(STAGING_DIR)/" \
	--with-libev-prefix="$(STAGING_DIR)/" \
	--without-lz4 \
	--with-local-talloc \

ifneq ($(CONFIG_OCSERV_PAM),y)
CONFIGURE_ARGS += --without-pam
endif

ifneq ($(CONFIG_OCSERV_RADIUS),y)
CONFIGURE_ARGS += --without-radius
endif

ifneq ($(CONFIG_OCSERV_SECCOMP),y)
CONFIGURE_ARGS += --disable-seccomp
endif

ifneq ($(CONFIG_OCSERV_PROTOBUF),y)
CONFIGURE_ARGS += --without-protobuf
endif

ifneq ($(CONFIG_OCSERV_HTTP_PARSER),y)
CONFIGURE_ARGS += --without-http-parser
endif

define Package/ocserv/conffiles
/etc/config/ocserv
endef

define Package/ocserv/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ocserv $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ocserv-worker $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ocserv-fw $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ocpasswd/ocpasswd $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/occtl/occtl $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ocserv.init $(1)/etc/init.d/ocserv
	$(INSTALL_DIR) $(1)/etc/ocserv
	$(INSTALL_CONF) ./files/ocserv.conf.template $(1)/etc/ocserv/ocserv.conf.template
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/config $(1)/etc/config/ocserv
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) ./files/ocserv.upgrade $(1)/lib/upgrade/keep.d/ocserv
endef

$(eval $(call BuildPackage,ocserv))
