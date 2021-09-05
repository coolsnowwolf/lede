include $(TOPDIR)/rules.mk

PKG_NAME:=leech
PKG_VERSION:=1.7
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://bitbucket.org/alekseyt/leech.git
PKG_SOURCE_DATE:=2019-07-17
PKG_SOURCE_VERSION:=2378e7348a8ac8ed23b6d29be81dc3a73080d920
PKG_MIRROR_HASH:=04ca1d1f2cf614edf871360c1adbc758d9b85b67296af438449367c068ce5320

PKG_MAINTAINER:=Leong Hui Wong <wong.leonghui@gmail.com>
PKG_LICENSE:=blessing
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/leech
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Download Manager
  TITLE:=RSS-feeds parser/files downloader
  URL:=https://bitbucket.org/alekseyt/leech/
  DEPENDS:=+xsltproc +curl
endef

define Package/leech/description
  leech downloads files for you if you give it RSS-feeds of your favorite
  trackers. Configuration files are under /etc/leech.
endef

define Build/Compile
endef

define Package/leech/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sbin/* $(1)/usr/sbin

	$(INSTALL_DIR) $(1)/etc/leech
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/config/* $(1)/etc/leech

	$(INSTALL_DIR) $(1)/usr/share/leech
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/share/leech/* $(1)/usr/share/leech
endef

define Package/leech/conffiles
/etc/leech/default
/etc/leech/downloads
/etc/leech/foods
/etc/leech/wild-downloads
/etc/leech/reverse-downloads
endef

$(eval $(call BuildPackage,leech))
