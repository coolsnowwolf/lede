include $(TOPDIR)/rules.mk

PKG_NAME:=libtorrent-rasterbar
PKG_VERSION:=2.0.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/arvidn/libtorrent/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=d5960e8c9f80f62126d723c6cc0522e7900c9c323f28994027eae3325fe03a3f

PKG_MAINTAINER:=David Yang <mmyangfl@gmail.com>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libtorrent-rasterbar/Default
	TITLE:=Rasterbar BitTorrent library
	URL:=https://libtorrent.org/
endef

define Package/libtorrent-rasterbar
	$(call Package/libtorrent-rasterbar/Default)
	SECTION:=libs
	CATEGORY:=Libraries
	DEPENDS:=+boost-system +libopenssl +libatomic
endef

#define Package/python3-libtorrent
#	$(call Package/libtorrent-rasterbar/Default)
#	SECTION:=lang
#	CATEGORY:=Languages
#	SUBMENU:=Python
#	TITLE+= (Python 3)
#	DEPENDS:=+libtorrent-rasterbar +boost-python
#endef

define Package/libtorrent-rasterbar/description
	Rasterbar libtorrent is a C++ library that aims to be a good alternative to
	all the other bittorrent implementations around.
endef

#define Package/python3-libtorrent/description
#	$(call Package/libtorrent-rasterbar/description)
#	This package contains Python 3 bindings for the libtorrent-rasterbar library.
#endef

define Download/try_signal
	VERSION:=334fd139e2bb387017b42d36753a03935e3bca75
	SUBDIR:=deps/try_signal
	FILE:=$(PKG_NAME)-try_signal-$$(VERSION).tar.xz
	URL:=https://github.com/arvidn/try_signal.git
	MIRROR_HASH:=c85d65352c20713cb6cfb005942b46ab7579dc7eef3b876666bff9347149d47f
	PROTO:=git
endef
$(eval $(call Download,try_signal))

PKG_UNPACK:=$(HOST_TAR) -C $(PKG_BUILD_DIR) --strip-components=1 -xzf $(DL_DIR)/$(PKG_SOURCE)

define Build/Prepare
	$(Build/Prepare/Default)
	$(eval $(Download/try_signal))
	xzcat $(DL_DIR)/$(FILE) | tar -C $(PKG_BUILD_DIR) $(TAR_OPTIONS)
endef

#CMAKE_OPTIONS += \
#	-Dpython-bindings=ON \
#	-Dpython-egg-info=ON

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libtorrent $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtorrent-rasterbar.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libtorrent-rasterbar.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libtorrent-rasterbar/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtorrent-rasterbar.so.* $(1)/usr/lib/
endef

#define Package/python3-libtorrent/install
#	$(INSTALL_DIR) $(1)/usr/lib/python2.7/site-packages
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/python2.7/site-packages/*.so* $(1)/usr/lib/python2.7/site-packages/
#endef

$(eval $(call BuildPackage,libtorrent-rasterbar))
#$(eval $(call BuildPackage,python3-libtorrent))
