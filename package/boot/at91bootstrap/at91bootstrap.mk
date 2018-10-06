
PKG_TARGETS := bin
PKG_FLAGS:=nonshared

export GCC_HONOUR_COPTS=s

define Package/at91bootstrap/install/default
  $(CP) -avL $(PKG_BUILD_DIR)/binaries/at91bootstrap.bin $(1)/
endef

Package/at91bootstrap/install = $(Package/at91bootstrap/install/default)

define AT91Bootstrap/Init
  BUILD_TARGET:=
  BUILD_SUBTARGET:=
  BUILD_DEVICES:=
  NAME:=
  DEPENDS:=
  HIDDEN:=
  DEFAULT:=
  VARIANT:=$(1)
  AT91BOOTSTRAP_CONFIG:=$(1)
endef

TARGET_DEP = TARGET_$(BUILD_TARGET)$(if $(BUILD_SUBTARGET),_$(BUILD_SUBTARGET))

AT91BOOTSTRAP_MAKE_FLAGS = \
	HOSTCC="$(HOSTCC)" \
	HOSTCFLAGS="$(HOST_CFLAGS) $(HOST_CPPFLAGS)" \
	HOSTLDFLAGS=""

define Build/AT91Bootstrap/Target
  $(eval $(call AT91Bootstrap/Init,$(1)))
  $(eval $(call AT91Bootstrap/Default,$(1)))
  $(eval $(call AT91Bootstrap/$(1),$(1)))

 define Package/at91bootstrap-$(1)
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:= .$(NAME)
    VARIANT:=$(VARIANT)
    DEPENDS:=@!IN_SDK $(DEPENDS)
    HIDDEN:=$(HIDDEN)
    ifneq ($(BUILD_TARGET),)
      DEPENDS += @$(TARGET_DEP)
      ifneq ($(BUILD_DEVICES),)
        DEFAULT := y if ($(TARGET_DEP)_Default \
		$(patsubst %,|| $(TARGET_DEP)_DEVICE_%,$(BUILD_DEVICES)) \
		$(patsubst %,|| $(patsubst TARGET_%,TARGET_DEVICE_%, \
           $(TARGET_DEP))_DEVICE_%,$(BUILD_DEVICES)))
      endif
    endif
    $(if $(DEFAULT),DEFAULT:=$(DEFAULT))
    URL:=https://www.at91.com/linux4sam/bin/view/Linux4SAM/AT91Bootstrap
  endef

  define Package/at91bootstrap-$(1)/install
    $$(Package/at91bootstrap/install)
  endef
endef

define Build/Configure/AT91Bootstrap
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
        $(AT91BOOTSTRAP_CONFIG)_defconfig
endef


define Build/Compile/AT91Bootstrap
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
       CROSS_COMPILE=$(TARGET_CROSS) \
       $(AT91BOOTSTRAP_MAKE_FLAGS)
endef

define BuildPackage/AT91Bootstrap/Defaults
  Build/Configure/Default = $$$$(Build/Configure/AT91Bootstrap)
  Build/Compile/Default = $$$$(Build/Compile/AT91Bootstrap)
endef

define BuildPackage/AT91Bootstrap
  $(eval $(call BuildPackage/AT91Bootstrap/Defaults))
  $(foreach type,$(if $(DUMP),$(AT91BOOTSTRAP_TARGETS),$(BUILD_VARIANT)), \
    $(eval $(call Build/AT91Bootstrap/Target,$(type)))
  )
  $(eval $(call Build/DefaultTargets))
  $(foreach type,$(if $(DUMP),$(AT91BOOTSTRAP_TARGETS),$(BUILD_VARIANT)), \
    $(call BuildPackage,at91bootstrap-$(type))
  )
endef
