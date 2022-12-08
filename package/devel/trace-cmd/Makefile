include $(TOPDIR)/rules.mk

PKG_NAME:=trace-cmd
PKG_VERSION:=v3.1.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/utils/trace-cmd/trace-cmd.git/snapshot/
PKG_HASH:=447e095dbdfb0d362ab8c2086d62d80c5a2ecf67aef09b8f6b0cc064c0e1bfb5

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/trace-cmd
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Linux trace command line utility
  DEPENDS:=+libtracefs +zlib
endef

define Package/trace-cmd-extra
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Extra plugins for trace-cmd
  DEPENDS:=+libtraceevent-extra
endef

MAKE_FLAGS += \
	NO_PYTHON=1 \
	NO_AUDIT=1 \
	NO_LIBZSTD=1 \
	prefix=/usr

TARGET_CFLAGS += --std=gnu99 -D_GNU_SOURCE

define Package/trace-cmd/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib/traceevent/plugins
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/trace-cmd $(1)/usr/bin/
endef

$(eval $(call BuildPackage,trace-cmd))
$(eval $(call BuildPackage,trace-cmd-extra))
