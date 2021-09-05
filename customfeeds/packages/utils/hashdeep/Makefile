include $(TOPDIR)/rules.mk

PKG_NAME:=hashdeep
PKG_VERSION:=4.4
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/jessek/hashdeep/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=ad78d42142f9a74fe8ec0c61bc78d6588a528cbb9aede9440f50b6ff477f3a7f

PKG_MAINTAINER:=Tobe Deprez <trldp@outlook.com>
PKG_LICENSE:=NLPL
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/hashdeep
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Recursively compute hashsums or piecewise hashings
  URL:=http://md5deep.sourceforge.net/
  DEPENDS:=+libstdcpp
endef

define Package/hashdeep/description
	hashdeep is a set of tools to compute MD5, SHA1, SHA256, tiger
	and whirlpool hashsums of arbitrary number of files recursively.
endef

define Package/hashdeep/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/hashdeep \
                   $(PKG_INSTALL_DIR)/usr/bin/md5deep \
                   $(PKG_INSTALL_DIR)/usr/bin/sha1deep \
                   $(PKG_INSTALL_DIR)/usr/bin/sha256deep \
                   $(PKG_INSTALL_DIR)/usr/bin/tigerdeep \
                   $(PKG_INSTALL_DIR)/usr/bin/whirlpooldeep \
                   $(1)/usr/bin/
endef

$(eval $(call BuildPackage,hashdeep))

