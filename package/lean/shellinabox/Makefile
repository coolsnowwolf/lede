
include $(TOPDIR)/rules.mk

PKG_NAME:=shellinabox
PKG_VERSION:=2.20
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/shellinabox/shellinabox.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=98e6eebc6c2026fb126a458c6cb5a2541447258e
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/shellinabox 
  SECTION:=net
  CATEGORY:=Network
  TITLE:=a web based terminal emulator
  URL:=https://github.com/shellinabox/shellinabox
  DEPENDS:=+zlib +libopenssl +busybox +@BUSYBOX_CUSTOM +@BUSYBOX_CONFIG_LOGIN
endef

define Package/shellinabox/description
	Shell In A Box implements a web server that 
	can export arbitrary command line tools to 
	a web based terminal emulator. This emulator 
	is accessible to any JavaScript and CSS 
	enabled web browser and does not require any additional browser plugins.
endef

CONFIGURE_ARGS += --disable-utmp

define Package/shellinabox/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/shellinabox
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/shellinaboxd $(1)/usr/sbin
	$(INSTALL_BIN) ./files/shellinaboxd.init $(1)/etc/init.d/shellinaboxd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/shellinabox/*.css $(1)/etc/shellinabox
	$(PKG_BUILD_DIR)/make-chained-cert.sh > $(1)/etc/shellinabox/certificate.pem
endef

$(eval $(call BuildPackage,shellinabox))