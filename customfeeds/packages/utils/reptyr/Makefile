include $(TOPDIR)/rules.mk

PKG_NAME:=reptyr
PKG_VERSION:=0.8.0
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/nelhage/reptyr/archive/
PKG_HASH:=4b470ed2a0d25fed591739fa9613ce7ad3d0377891eb56cbe914e3c85db46ca8
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Josef Schlehofer <josef.schlehofer@nic.cz>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/reptyr
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Tool for reparenting running programs
  URL:=https://github.com/nelhage/reptyr
  DEPENDS:=@!(arc||mips||mipsel||mips64||mips64el)
endef

define Package/reptyr/description
  reptyr is a utility for taking an existing running program and attaching it
  to a new terminal. Started a long-running process over ssh, but have to
  leave and don't want to interrupt it? Just start a screen, use reptyr to
  grab it, and then kill the ssh session and head on home.
endef

MAKE_FLAGS+= \
	PREFIX=/usr

define Package/reptyr/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/reptyr $(1)/usr/bin/
endef

$(eval $(call BuildPackage,reptyr))
