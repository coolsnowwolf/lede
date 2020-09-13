PKG_NAME ?= u-boot

ifndef PKG_SOURCE_PROTO
PKG_SOURCE = $(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL = \
	https://mirror.cyberbits.eu/u-boot \
	https://ftp.denx.de/pub/u-boot \
	ftp://ftp.denx.de/pub/u-boot
endif

PKG_BUILD_DIR = $(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_TARGETS := bin
PKG_FLAGS:=nonshared

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_BUILD_PARALLEL:=1

export GCC_HONOUR_COPTS=s

define Package/u-boot/install/default
	$(CP) $(patsubst %,$(PKG_BUILD_DIR)/%,$(UBOOT_IMAGE)) $(1)/
endef

Package/u-boot/install = $(Package/u-boot/install/default)

define U-Boot/Init
  BUILD_TARGET:=
  BUILD_SUBTARGET:=
  BUILD_DEVICES:=
  NAME:=
  DEPENDS:=
  HIDDEN:=
  DEFAULT:=
  VARIANT:=$(1)
  UBOOT_CONFIG:=$(1)
  UBOOT_IMAGE:=u-boot.bin
endef

TARGET_DEP = TARGET_$(BUILD_TARGET)$(if $(BUILD_SUBTARGET),_$(BUILD_SUBTARGET))

UBOOT_MAKE_FLAGS = \
	HOSTCC="$(HOSTCC)" \
	HOSTCFLAGS="$(HOST_CFLAGS) $(HOST_CPPFLAGS) -std=gnu11" \
	HOSTLDFLAGS="$(HOST_LDFLAGS)"

define Build/U-Boot/Target
  $(eval $(call U-Boot/Init,$(1)))
  $(eval $(call U-Boot/Default,$(1)))
  $(eval $(call U-Boot/$(1),$(1)))

 define Package/u-boot-$(1)
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=U-Boot for $(NAME)
    VARIANT:=$(VARIANT)
    DEPENDS:=@!IN_SDK $(DEPENDS)
    HIDDEN:=$(HIDDEN)
    ifneq ($(BUILD_TARGET),)
      DEPENDS += @$(TARGET_DEP)
      ifneq ($(BUILD_DEVICES),)
        DEFAULT := y if ($(TARGET_DEP)_Default \
		$(patsubst %,|| $(TARGET_DEP)_DEVICE_%,$(BUILD_DEVICES)) \
		$(patsubst %,|| $(patsubst TARGET_%,TARGET_DEVICE_%,$(TARGET_DEP))_DEVICE_%,$(BUILD_DEVICES)))
      endif
    endif
    $(if $(DEFAULT),DEFAULT:=$(DEFAULT))
    URL:=http://www.denx.de/wiki/U-Boot
  endef

  define Package/u-boot-$(1)/install
	$$(Package/u-boot/install)
  endef
endef

define Build/Configure/U-Boot
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) $(UBOOT_CONFIGURE_VARS) $(UBOOT_CONFIG)_config
endef

DTC=$(wildcard $(LINUX_DIR)/scripts/dtc/dtc)

define Build/Compile/U-Boot
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		$(if $(DTC),DTC="$(DTC)") \
		$(UBOOT_MAKE_FLAGS)
endef

define BuildPackage/U-Boot/Defaults
  Build/Configure/Default = $$$$(Build/Configure/U-Boot)
  Build/Compile/Default = $$$$(Build/Compile/U-Boot)
endef

define BuildPackage/U-Boot
  $(eval $(call BuildPackage/U-Boot/Defaults))
  $(foreach type,$(if $(DUMP),$(UBOOT_TARGETS),$(BUILD_VARIANT)), \
    $(eval $(call Build/U-Boot/Target,$(type)))
  )
  $(eval $(call Build/DefaultTargets))
  $(foreach type,$(if $(DUMP),$(UBOOT_TARGETS),$(BUILD_VARIANT)), \
    $(call BuildPackage,u-boot-$(type))
  )
endef
