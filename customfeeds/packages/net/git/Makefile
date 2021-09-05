#
# Copyright (C) 2009-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=git
PKG_VERSION:=2.30.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/software/scm/git/
PKG_HASH:=41f7d90c71f9476cd387673fcb10ce09ccbed67332436a4cc58d7af32c355faa

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:git-scm:git

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/git/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Version Control Systems
  DEPENDS:=+libopenssl +libpthread +librt +zlib
  URL:=https://git-scm.com
  MAINTAINER:=Peter Wagner <tripolar@gmx.at>
endef

define Package/git
$(call Package/git/Default)
  TITLE:=The fast version control system
endef

define Package/git/description
 Git is a free & open source, distributed version control system
 designed to handle everything from small to very large projects
 with speed and efficiency.
endef

define Package/git-http
$(call Package/git/Default)
  DEPENDS+= +git +libcurl
  TITLE:=Git HTTP commands
endef

define Package/git-http/description
$(call Package/git/description)

 This package allows git push/fetch over http(s) and ftp(s)
endef

define Package/git-gitweb
$(call Package/git/Default)
  TITLE:=Git repository web interface
  DEPENDS+=	+git \
		+perl-cgi \
		+perlbase-digest \
		+perlbase-encode \
		+perlbase-essential \
		+perlbase-fcntl \
		+perlbase-file \
		+perlbase-filetest \
		+perlbase-storable \
		+perlbase-time
endef

define Package/git-gitweb/description
$(call Package/git/description)
 This package builds the gitweb web interface for git repositories
endef

define Package/git-gitweb/conffiles
/etc/gitweb.conf
endef

MAKE_FLAGS := \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	CPPFLAGS="$(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	NO_EXPAT="YesPlease" \
	NO_MKSTEMPS="YesPlease" \
	NO_GETTEXT="YesPlease" \
	NO_UNIX_SOCKETS="YesPlease" \
	NO_ICONV="YesPlease" \
	NO_NSEC="YesPlease" \
	NO_PERL="YesPlease" \
	NO_PYTHON="YesPlease" \
	NO_TCLTK="YesPlease" \
	NO_INSTALL_HARDLINKS="yes" \
	gitwebdir="/www/cgi-bin" \
	GITWEB_JS="/gitweb/gitweb.js" \
	GITWEB_CSS="/gitweb/gitweb.css" \
	GITWEB_LOGO="/gitweb/gitweb-logo.png" \
	GITWEB_FAVICON="/gitweb/gitweb-favicon.png" \
	uname_S="Linux" \

CONFIGURE_ARGS += \
	--without-iconv \

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) \
		configure

	$(call Build/Configure/Default,)
endef

define Build/Compile
	mkdir -p $(PKG_INSTALL_DIR)/www/cgi-bin $(PKG_INSTALL_DIR)/www/gitweb
	$(call Build/Compile/Default,DESTDIR=$(PKG_INSTALL_DIR) all install gitweb install-gitweb)
endef

define Package/git/install
	$(RM) $(PKG_INSTALL_DIR)/usr/bin/git-cvsserver
	$(RM) $(PKG_INSTALL_DIR)/usr/bin/git-shell
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/git-* $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib/git-core
	$(INSTALL_DIR) $(1)/usr/share/git-core/templates
	( cd $(PKG_INSTALL_DIR); $(TAR) \
		--exclude=usr/lib/git-core/git-http-backend \
		--exclude=usr/lib/git-core/git-http-fetch \
		--exclude=usr/lib/git-core/git-remote-ftp \
		--exclude=usr/lib/git-core/git-remote-ftps \
		--exclude=usr/lib/git-core/git-remote-http \
		--exclude=usr/lib/git-core/git-remote-https \
		-cf - \
		usr/lib/git-core \
		usr/share/git-core/templates \
	) | ( cd $(1); $(TAR) -xf - )
	ln $(1)/usr/lib/git-core/git $(1)/usr/bin/git
	ln $(1)/usr/lib/git-core/git-shell $(1)/usr/bin/git-shell
endef

define Package/git-http/install
	$(INSTALL_DIR) $(1)/usr/lib/git-core
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/git-core/git-http-backend $(1)/usr/lib/git-core
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/git-core/git-http-fetch $(1)/usr/lib/git-core
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/git-core/git-remote-ftp $(1)/usr/lib/git-core
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/git-core/git-remote-ftps $(1)/usr/lib/git-core
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/git-core/git-remote-http $(1)/usr/lib/git-core
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/git-core/git-remote-https $(1)/usr/lib/git-core
endef

define Package/git-gitweb/install
	$(INSTALL_DIR) $(1)/www/cgi-bin $(1)/www/gitweb
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/www/cgi-bin/gitweb.cgi $(1)/www/cgi-bin/
	$(CP) $(PKG_INSTALL_DIR)/www/cgi-bin/static/* $(1)/www/gitweb/
endef

$(eval $(call BuildPackage,git))
$(eval $(call BuildPackage,git-http))
$(eval $(call BuildPackage,git-gitweb))
