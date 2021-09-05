#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vim
PKG_VERSION:=8.2
PKG_RELEASE:=4
VIMVER:=82

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://ftp.vim.org/pub/vim/unix
PKG_HASH:=f087f821831b4fece16a0461d574ccd55a8279f64d635510a1e10225966ced3b
PKG_MAINTAINER:=Marko Ratkaj <marko.ratkaj@sartura.hr>
PKG_CPE_ID:=cpe:/a:vim:vim

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)$(VIMVER)
PKG_BUILD_PARALLEL:=1

HOST_BUILD_DEPENDS:=libiconv/host
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)$(VIMVER)
HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/vim/Default
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libncurses
  TITLE:=Vi IMproved - enhanced vi editor
  URL:=http://www.vim.org/
  SUBMENU:=Editors
endef

define Package/vim
  $(call Package/vim/Default)
  TITLE+= (Tiny)
endef

define Package/vim-full
  $(call Package/vim/Default)
  TITLE+= (Normal)
endef


define Package/vim-fuller
  $(call Package/vim/Default)
  TITLE+= (Big)
endef

define Package/vim-runtime
  $(call Package/vim/Default)
  TITLE+= (runtime files)
endef

define Package/vim-help
  $(call Package/vim/Default)
  TITLE+= (help files)
endef

define Package/xxd
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=make a hexdump or do the reverse
  URL:=http://www.vim.org/
endef

define Package/vim-full/conffiles
/usr/share/vim/vimrc
/root/.vimrc
endef

define Package/vim/conffiles
/usr/share/vim/vimrc
/root/.vimrc
endef

define Package/vim/description
 Vim is an almost compatible version of the UNIX editor Vi.
 (Tiny build)
endef

define Package/vim-full/description
 Vim is an almost compatible version of the UNIX editor Vi.
 (Normal build)
endef


define Package/vim-fuller/description
 Vim is an almost compatible version of the UNIX editor Vi.
 (Big build)
endef

define Package/vim-runtime/description
 Vim is an almost compatible version of the UNIX editor Vi.
 (Runtime files)
endef

define Package/vim-help/description
 Vim is an almost compatible version of the UNIX editor Vi.
 (Help files)
endef

define Package/xxd/description
 xxd creates a hex dump of a given file or standard input, it can also convert
 a hex dump back to its original binary form.
endef

HOST_CONFIGURE_ARGS += \
	--disable-acl \
	--disable-gpm \
	--disable-nls \
	--disable-selinux \
	--enable-gui=no \
	--with-features=normal \
	--without-x

CONFIGURE_ARGS += \
	--disable-gui \
	--disable-gtktest \
	--disable-xim \
	--without-x \
	--disable-netbeans \
	--disable-cscope \
	--disable-gpm \
	--disable-acl \
	--disable-selinux \
	--with-tlib=ncurses \
	--with-compiledby="non-existent-hostname-compiled"

CONFIGURE_VARS += \
	ac_cv_header_elf_h=no \
	vim_cv_getcwd_broken=no \
	vim_cv_memmove_handles_overlap=yes \
	vim_cv_stat_ignores_slash=yes \
	vim_cv_tgetent=zero \
	vim_cv_terminfo=yes \
	vim_cv_toupper_broken=no \
	vim_cv_tty_group=root \
	vim_cv_tty_mode=0620

ifneq ($(HOST_OS),Linux)
  TARGET_PATH_PKG:=$(CURDIR)/scripts:$(TARGET_PATH_PKG)
endif

define Build/Prepare
	$(call Build/Prepare/Default)
	$(MAKE) -C $(PKG_BUILD_DIR)/src autoconf
endef

ifneq ($(CONFIG_PACKAGE_vim),)
define Build/Compile/vim
	$(call Build/Configure/Default, \
		--with-features=tiny \
	)
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" all
	$(CP) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(PKG_BUILD_DIR)/vim_tiny
endef
endif

ifneq ($(CONFIG_PACKAGE_vim-full),)
define Build/Compile/vim-full
	$(call Build/Configure/Default, \
		--with-features=normal \
	)
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" all
	$(CP) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(PKG_BUILD_DIR)/vim_normal
endef
endif

ifneq ($(CONFIG_PACKAGE_vim-fuller),)
define Build/Compile/vim-fuller
	$(call Build/Configure/Default, \
		--with-features=big \
	)
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" all
	$(CP) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(PKG_BUILD_DIR)/vim_big
endef
endif



ifneq ($(CONFIG_PACKAGE_xxd),)
define Build/Compile/xxd
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" all
endef
endif

define Build/Compile/vim-runtime
	$(MAKE) -C $(PKG_BUILD_DIR)/src DESTDIR="$(PKG_INSTALL_DIR)" installrtbase
	(cd $(PKG_INSTALL_DIR) && tar -cf $(PKG_BUILD_DIR)/docs.tar ./usr/share/vim/vim$(VIMVER)/doc)
	rm -rf $(PKG_INSTALL_DIR)/usr/share/vim/vim$(VIMVER)/doc
	rm -rf $(PKG_INSTALL_DIR)/usr/man
endef

define Build/Compile
$(call Build/Compile/vim)
$(call Build/Compile/vim-full)
$(call Build/Compile/vim-fuller)
$(call Build/Compile/vim-runtime)
$(call Build/Compile/xxd)
endef

define Package/vim/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vim_tiny $(1)/usr/bin/vim
	$(INSTALL_DIR) $(1)/usr/share/vim
	$(INSTALL_CONF) ./files/vimrc $(1)/usr/share/vim/
endef

define Package/vim-full/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vim_normal $(1)/usr/bin/vim
	$(INSTALL_DIR) $(1)/usr/share/vim
	$(LN) vim $(1)/usr/bin/vimdiff
	$(INSTALL_CONF) ./files/vimrc.full $(1)/usr/share/vim/vimrc
endef


define Package/vim-fuller/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vim_big $(1)/usr/bin/vim
	$(INSTALL_DIR) $(1)/usr/share/vim
	$(LN) vim $(1)/usr/bin/vimdiff
	$(CP) $(PKG_INSTALL_DIR)/usr/share/vim/vim$(VIMVER) $(1)/usr/share/vim
	$(INSTALL_CONF) ./files/vimrc.full $(1)/usr/share/vim/vimrc
endef



define Package/vim-runtime/install
	$(CP) $(PKG_INSTALL_DIR)/* $(1)
	rm -rf $(1)/usr/share/vim/vim$(VIMVER)/doc
endef

define Package/vim-help/install
	tar -C $(1) -xf $(PKG_BUILD_DIR)/docs.tar
endef

define Package/xxd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/xxd/xxd $(1)/usr/bin
endef

$(eval $(call BuildPackage,vim))
$(eval $(call BuildPackage,vim-full))
$(eval $(call BuildPackage,vim-fuller))
$(eval $(call BuildPackage,vim-runtime))
$(eval $(call BuildPackage,vim-help))
$(eval $(call BuildPackage,xxd))
$(eval $(call HostBuild))
