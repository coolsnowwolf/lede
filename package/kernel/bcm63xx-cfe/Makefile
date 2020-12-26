include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=bcm63xx-cfe
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://github.com/openwrt/bcm63xx-cfe.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-05-18
PKG_SOURCE_VERSION:=efe3b81121a84c0b8ba6a7d0d47cd5eeeb23808d
PKG_MIRROR_HASH:=299dcf6ef1ad034df26daee6446b574abcd7526a2fe90fb8115890e71bc0d58b

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/bcm63xx-cfe
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_bcm63xx
  TITLE:=bcm63xx-cfe
  DEFAULT:=y if TARGET_bcm63xx
endef

define Package/bcm63xx-cfe/description
  CFE RAM binaries for bcm63xx.
endef

define Build/Compile
	true
endef

define Package/bcm63xx-cfe/install
	true
endef

define Build/InstallDev
	rm -rf $(KERNEL_BUILD_DIR)/$(PKG_NAME)
	mkdir -p $(KERNEL_BUILD_DIR)/$(PKG_NAME)
	$(CP) -r $(PKG_BUILD_DIR)/* $(KERNEL_BUILD_DIR)/$(PKG_NAME)
endef

$(eval $(call BuildPackage,bcm63xx-cfe))
