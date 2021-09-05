include $(TOPDIR)/rules.mk

PKG_NAME:=gnuplot
PKG_VERSION:=5.4.1
PKG_RELEASE:=1
PKG_MAINTAINER:=Matteo Cicuttin <datafl4sh@toxicnet.eu>

PKG_BUILD_DIR:=$(BUILD_DIR)/gnuplot-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/gnuplot
PKG_HASH:=6b690485567eaeb938c26936e5e0681cf70c856d273cc2c45fabf64d8bc6590e
PKG_CAT:=zcat
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/gnuplot
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+zlib +libgd +libncursesw +libreadline
  TITLE:=A portable command-line driven graphing utility
  URL:=http://www.gnuplot.info/
endef

CONFIGURE_ARGS += \
    --without-cairo \
    --with-qt=no \
    --without-lua \
    --without-latex \
    --without-libcerf

# Since Qt is disabled there is no C++ code, so CXX is forced
# to CC in order to avoid dependency on libstdcpp. This is 
# horrible, but 'DEPENDS:=+libstdcpp' does not work for some
# reason. 
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CXX="$(TARGET_CC_NOCACHE)"  
endef

define Package/gnuplot/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/gnuplot $(1)/usr/bin
endef

$(eval $(call BuildPackage,gnuplot))

