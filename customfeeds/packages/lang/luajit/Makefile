include $(TOPDIR)/rules.mk

PKG_NAME:=luajit
PKG_VERSION:=2.1.0-beta3
PKG_RELEASE:=5

PKG_SOURCE:=LuaJIT-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://luajit.org/download
PKG_HASH:=1ad2e34b111c802f9d0cdf019e986909123237a28c746b21295b63c9e785d9c3
PKG_BUILD_DIR:=$(BUILD_DIR)/LuaJIT-$(PKG_VERSION)

PKG_MAINTAINER:=Morteza Milani <milani@pichak.co>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYRIGHT

PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/luajit
 SUBMENU:=Lua
 SECTION:=lang
 CATEGORY:=Languages
 TITLE:=LuaJIT
 URL:=https://www.luajit.org
 DEPENDS:=@(i386||x86_64||arm||armeb||aarch64||(powerpc&&HAS_FPU)||mips||mipsel||mips64)
endef

define Package/luajit/description
 LuaJIT is a Just-In-Time (JIT) compiler for the Lua programming language. *** Requires GCC Multilib on host system to build! ***
endef
ifeq ($(HOST_ARCH),$(filter $(HOST_ARCH), x86_64 mips64))
  ifeq ($(CONFIG_ARCH_64BIT),)
    HOST_BITS := -m32
  endif
endif

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		HOST_CC="$(HOSTCC) $(HOST_CFLAGS) $(HOST_BITS)" \
		CROSS="$(TARGET_CROSS)" \
		DPREFIX=$(PKG_INSTALL_DIR)/usr \
		PREFIX=/usr \
		TARGET_SYS=Linux \
		TARGET_CFLAGS="$(TARGET_CFLAGS)" \
		BUILDMODE=dynamic
	rm -rf $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DPREFIX=$(PKG_INSTALL_DIR)/usr \
		PREFIX=/usr \
		TARGET_SYS=Linux \
		install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/luajit-2.1
	$(CP) $(PKG_INSTALL_DIR)/usr/include/luajit-2.1/*.{h,hpp} $(1)/usr/include/luajit-2.1
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/luajit.pc $(1)/usr/lib/pkgconfig/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/luajit-$(PKG_VERSION) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME)
endef

define Package/luajit/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/luajit-$(PKG_VERSION) $(1)/usr/bin/$(PKG_NAME)
endef

define Host/Compile
	$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) \
		DPREFIX=$(STAGING_DIR_HOSTPKG) \
		TARGET_CFLAGS="$(HOST_CFLAGS)" \
		TARGET_LDFLAGS="$(HOST_LDFLAGS)"
endef

define Host/Install
	$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) \
		DPREFIX=$(STAGING_DIR_HOSTPKG) \
		install
	$(CP) $(STAGING_DIR_HOSTPKG)/bin/luajit-$(PKG_VERSION) $(STAGING_DIR_HOSTPKG)/bin/$(PKG_NAME)
endef

$(eval $(call HostBuild,luajit))
$(eval $(call BuildPackage,luajit))
