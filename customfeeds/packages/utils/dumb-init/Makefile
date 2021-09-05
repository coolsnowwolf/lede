include $(TOPDIR)/rules.mk

PKG_NAME:=dumb-init
PKG_VERSION:=1.2.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/Yelp/dumb-init/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=d4e2e10e39ad49c225e1579a4d770b83637399a0be48e29986f720fae44dafdf
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Toni Uhlig <matzeton@googlemail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

MAKE_FLAGS+=CFLAGS='$(TARGET_CFLAGS) $(TARGET_LDFLAGS)'

define Package/dumb-init
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=dumb-init
  URL:=https://github.com/Yelp/dumb-init.git
endef

define Package/dumb-init/description
  dumb-init is a simple process supervisor and init system designed to run as
  PID 1 inside minimal container environments.
endef

define Package/dumb-init/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dumb-init $(1)/usr/sbin/dumb-init
endef

$(eval $(call BuildPackage,dumb-init))
