PKG_NAME ?= trusted-firmware-a
PKG_CPE_ID ?= cpe:/a:arm:trusted_firmware-a

ifndef PKG_SOURCE_PROTO
PKG_SOURCE = trusted-firmware-a-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/TrustedFirmware-A/trusted-firmware-a/tar.gz/v$(PKG_VERSION)?
endif

PKG_BUILD_DIR = $(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_TARGETS := bin
PKG_FLAGS:=nonshared

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=docs/license.rst

PKG_BUILD_PARALLEL:=1

export GCC_HONOUR_COPTS=s

define Package/trusted-firmware-a/install/default
	$(CP) $(patsubst %,$(PKG_BUILD_DIR)/build/$(PLAT)/release/%,$(TFA_IMAGE)) $(1)/
endef

Package/trusted-firmware-a/install = $(Package/trusted-firmware-a/install/default)

define Trusted-Firmware-A/Init
  BUILD_TARGET:=
  BUILD_SUBTARGET:=
  BUILD_DEVICES:=
  NAME:=
  DEPENDS:=
  HIDDEN:=
  DEFAULT:=
  PLAT:=
  VARIANT:=$(1)
  TFA_IMAGE:=
endef

TARGET_DEP = TARGET_$(BUILD_TARGET)$(if $(BUILD_SUBTARGET),_$(BUILD_SUBTARGET))

define Build/Trusted-Firmware-A/Target
  $(eval $(call Trusted-Firmware-A/Init,$(1)))
  $(eval $(call Trusted-Firmware-A/Default,$(1)))
  $(eval $(call Trusted-Firmware-A/$(1),$(1)))

 define Package/trusted-firmware-a-$(1)
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=Trusted-Firmware-A for $(NAME)
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
    URL:=https://www.trustedfirmware.org/projects/tf-a/
  endef

  define Package/trusted-firmware-a-$(1)/install
	$$(Package/trusted-firmware-a/install)
  endef
endef

define Build/Configure/Trusted-Firmware-A
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include
endef

DTC=$(wildcard $(LINUX_DIR)/scripts/dtc/dtc)

define Build/Compile/Trusted-Firmware-A
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		OPENSSL_DIR=$(STAGING_DIR_HOST) \
		$(if $(DTC),DTC="$(DTC)") \
		PLAT=$(PLAT) \
		BUILD_STRING="OpenWrt v$(PKG_VERSION)-$(PKG_RELEASE) ($(VARIANT))" \
		$(if $(CONFIG_BINUTILS_VERSION_2_37)$(CONFIG_BINUTILS_VERSION_2_38),,LDFLAGS="-no-warn-rwx-segments") \
		$(TFA_MAKE_FLAGS)
endef

define BuildPackage/Trusted-Firmware-A/Defaults
  Build/Configure/Default = $$$$(Build/Configure/Trusted-Firmware-A)
  Build/Compile/Default = $$$$(Build/Compile/Trusted-Firmware-A)
endef

define BuildPackage/Trusted-Firmware-A
  $(eval $(call BuildPackage/Trusted-Firmware-A/Defaults))
  $(foreach type,$(if $(DUMP),$(TFA_TARGETS),$(BUILD_VARIANT)), \
    $(eval $(call Build/Trusted-Firmware-A/Target,$(type)))
  )
  $(eval $(call Build/DefaultTargets))
  $(foreach type,$(if $(DUMP),$(TFA_TARGETS),$(BUILD_VARIANT)), \
    $(call BuildPackage,trusted-firmware-a-$(type))
  )
endef
