#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=talloc
PKG_VERSION:=2.3.1
MAJOR_VERSION:=2
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.samba.org/ftp/talloc
PKG_HASH:=ef4822d2fdafd2be8e0cabc3ec3c806ae29b8268e932c5e9a4cd5585f37f9f77

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=LGPL-3.0-or-later

PKG_BUILD_PARALLEL:=0
PKG_BUILD_DEPENDS:=python3/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/libtalloc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Core memory allocator used in Samba
  DEPENDS:=$(ICONV_DEPENDS) +libattr
  URL:=https://talloc.samba.org/talloc/doc/html/index.html
endef

define Package/libtalloc/description
  talloc is a hierarchical, reference counted memory pool system with destructors.
endef

CROSS = " \
	\nChecking simple C program: \"hello world\" \
	\nrpath library support: (127, \"\") \
	\n-Wl,--version-script support: (127, \"\") \
	\nChecking getconf LFS_CFLAGS: NO \
	\nChecking for large file support without additional flags: OK \
	\nChecking correct behavior of strtoll: OK \
	\nChecking for working strptime: NO \
	\nChecking for C99 vsnprintf: \"1\" \
	\nChecking for HAVE_SHARED_MMAP: NO \
	\nChecking for HAVE_MREMAP: NO \
	\nChecking for HAVE_INCOHERENT_MMAP: (2, \"\") \
	\nChecking for HAVE_SECURE_MKSTEMP: OK \
	\n"

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		echo -e >cache.txt $(CROSS) " \
			\nChecking uname machine type: \"$(ARCH)\" \
			\nChecking uname release type: \"$(LINUX_VERSION)\" \
			\nChecking uname sysname type: \"Linux\" \
			\nChecking uname version type: \"$(LINUX_UNAME_VERSION)\" \
		\n" ; \
		$(CONFIGURE_VARS) \
		./buildtools/bin/waf configure \
			--prefix=/usr \
			--sysconfdir=/etc \
			--localstatedir=/var \
			--with-libiconv="$(ICONV_PREFIX)" \
			--cross-compile \
			--cross-answers=cache.txt \
			--disable-python \
			--disable-rpath \
			--disable-rpath-install \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/bin/default/libtalloc.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/bin/default/talloc.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libtalloc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/bin/default/libtalloc.so $(1)/usr/lib/libtalloc.so.$(MAJOR_VERSION)
	(cd $(1)/usr/lib; ln -sf libtalloc.so.$(MAJOR_VERSION) libtalloc.so)
endef

$(eval $(call BuildPackage,libtalloc))
