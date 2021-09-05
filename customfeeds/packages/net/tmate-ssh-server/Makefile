include $(TOPDIR)/rules.mk

PKG_NAME:=tmate-ssh-server
PKG_VERSION:=511fd2bd852464e76824279609a34ee93fe148a4
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/tmate-io/tmate-ssh-server/tar.gz/$(PKG_VERSION)?
PKG_HASH:=68602496ca6a17ea80f5be53eba047897ac714c7cdfcb3bcdc12c56b8f3c3b45

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Paul Spooren <mail@aparcar.org>

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tmate-ssh-server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Instant Terminal Sharing Server
  URL:=https://tmate.io
  DEPENDS:=+libevent2 +libncurses +libpthread +libssh +msgpack-c +terminfo +openssh-keygen
endef

define Package/tmate-ssh-server/description
  tmate-ssh-server is the server side part of tmate.io.
endef

CONFIGURE_VARS+= \
	LIBSSH_CFLAGS="-I$(STAGING_DIR)/usr/include" \
	LIBSSH_LIBS="-lssh"

TARGET_CFLAGS+= \
	-D_GNU_SOURCE \

define Package/tmate-ssh-server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tmate-ssh-server $(1)/usr/bin/tmate-ssh-server

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/tmate-ssh-server.init $(1)/etc/init.d/tmate-ssh-server

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/tmate-ssh-server.config $(1)/etc/config/tmate-ssh-server
endef

$(eval $(call BuildPackage,tmate-ssh-server))
