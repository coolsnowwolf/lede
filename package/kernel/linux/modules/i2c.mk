#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

I2C_MENU:=I2C support

ModuleConfVar=$(word 1,$(subst :,$(space),$(1)))
ModuleFullPath=$(LINUX_DIR)/$(word 2,$(subst :,$(space),$(1))).ko
ModuleKconfig=$(foreach mod,$(1),$(call ModuleConfVar,$(mod)))
ModuleFiles=$(foreach mod,$(1),$(call ModuleFullPath,$(mod)))
ModuleAuto=$(call AutoLoad,$(1),$(foreach mod,$(2),$(basename $(notdir $(call ModuleFullPath,$(mod))))),$(3))

define i2c_defaults
  SUBMENU:=$(I2C_MENU)
  KCONFIG:=$(call ModuleKconfig,$(1))
  FILES:=$(call ModuleFiles,$(1))
  AUTOLOAD:=$(call ModuleAuto,$(2),$(1),$(3))
endef

I2C_CORE_MODULES:= \
  CONFIG_I2C:drivers/i2c/i2c-core \
  CONFIG_I2C_CHARDEV:drivers/i2c/i2c-dev

define KernelPackage/i2c-core
  $(call i2c_defaults,$(I2C_CORE_MODULES),51)
  TITLE:=I2C support
endef

define KernelPackage/i2c-core/description
 Kernel modules for I2C support
endef

$(eval $(call KernelPackage,i2c-core))


I2C_ALGOBIT_MODULES:= \
  CONFIG_I2C_ALGOBIT:drivers/i2c/algos/i2c-algo-bit

define KernelPackage/i2c-algo-bit
  $(call i2c_defaults,$(I2C_ALGOBIT_MODULES),55)
  TITLE:=I2C bit-banging interfaces
  DEPENDS:=+kmod-i2c-core
endef

define KernelPackage/i2c-algo-bit/description
 Kernel modules for I2C bit-banging interfaces
endef

$(eval $(call KernelPackage,i2c-algo-bit))


I2C_ALGOPCA_MODULES:= \
  CONFIG_I2C_ALGOPCA:drivers/i2c/algos/i2c-algo-pca

define KernelPackage/i2c-algo-pca
  $(call i2c_defaults,$(I2C_ALGOPCA_MODULES),55)
  TITLE:=I2C PCA 9564 interfaces
  DEPENDS:=+kmod-i2c-core
endef

define KernelPackage/i2c-algo-pca/description
 Kernel modules for I2C PCA 9564 interfaces
endef

$(eval $(call KernelPackage,i2c-algo-pca))


I2C_ALGOPCF_MODULES:= \
  CONFIG_I2C_ALGOPCF:drivers/i2c/algos/i2c-algo-pcf

define KernelPackage/i2c-algo-pcf
  $(call i2c_defaults,$(I2C_ALGOPCF_MODULES),55)
  TITLE:=I2C PCF 8584 interfaces
  DEPENDS:=+kmod-i2c-core
endef

define KernelPackage/i2c-algo-pcf/description
 Kernel modules for I2C PCF 8584 interfaces
endef

$(eval $(call KernelPackage,i2c-algo-pcf))


I2C_CCGS_UCSI_MODULES:= \
  CONFIG_I2C_CCGX_UCSI:drivers/i2c/busses/i2c-ccgx-ucsi

define KernelPackage/i2c-ccgs-ucsi
  $(call i2c_defaults,$(I2C_CCGS_UCSI_MODULES),58)
  TITLE:=Cypress CCGx Type-C controller
  DEPENDS:=+kmod-i2c-core +kmod-regmap-core
  HIDDEN:=y
endef


$(eval $(call KernelPackage,i2c-ccgs-ucsi))


I2C_DWCORE_MODULES:= \
  CONFIG_I2C_DESIGNWARE_CORE:drivers/i2c/busses/i2c-designware-core

define KernelPackage/i2c-designware-core
  $(call i2c_defaults,$(I2C_DWCORE_MODULES),58)
  TITLE:=Synopsys DesignWare I2C core
  DEPENDS:=+kmod-i2c-core +!LINUX_5_4:kmod-regmap-core
  HIDDEN:=y
endef

$(eval $(call KernelPackage,i2c-designware-core))


I2C_DWPCI_MODULES:= \
  CONFIG_I2C_DESIGNWARE_PCI:drivers/i2c/busses/i2c-designware-pci

define KernelPackage/i2c-designware-pci
  $(call i2c_defaults,$(I2C_DWPCI_MODULES),59)
  TITLE:=Synopsys DesignWare PCI
  DEPENDS:=@PCI_SUPPORT +kmod-i2c-designware-core +kmod-i2c-ccgs-ucsi
endef

define KernelPackage/i2c-designware-pci/description
 Support for Synopsys DesignWare I2C controller. Only master mode is supported.
endef

$(eval $(call KernelPackage,i2c-designware-pci))


I2C_GPIO_MODULES:= \
  CONFIG_I2C_GPIO:drivers/i2c/busses/i2c-gpio

define KernelPackage/i2c-gpio
  $(call i2c_defaults,$(I2C_GPIO_MODULES),59)
  TITLE:=GPIO-based bitbanging I2C
  DEPENDS:=@GPIO_SUPPORT +kmod-i2c-algo-bit
endef

define KernelPackage/i2c-gpio/description
 Kernel modules for a very simple bitbanging I2C driver utilizing the
 arch-neutral GPIO API to control the SCL and SDA lines.
endef

$(eval $(call KernelPackage,i2c-gpio))


I2C_I801_MODULES:= \
  CONFIG_I2C_I801:drivers/i2c/busses/i2c-i801

define KernelPackage/i2c-i801
  $(call i2c_defaults,$(I2C_I801_MODULES),59)
  TITLE:=Intel I801 and compatible I2C interfaces
  DEPENDS:=@PCI_SUPPORT @TARGET_x86 +kmod-i2c-core +kmod-i2c-smbus
endef

define KernelPackage/i2c-i801/description
 Support for the Intel I801 family of mainboard I2C interfaces,
 specifically 82801AA, 82801AB, 82801BA, 82801CA/CAM, 82801DB,
 82801EB/ER (ICH5/ICH5R), 6300ESB, ICH6, ICH7, ESB2, ICH8, ICH9,
 EP80579 (Tolapai), ICH10, 5/3400 Series (PCH), 6 Series (PCH),
 Patsburg (PCH), DH89xxCC (PCH), Panther Point (PCH),
 Lynx Point (PCH), Lynx Point-LP (PCH), Avoton (SOC),
 Wellsburg (PCH), Coleto Creek (PCH), Wildcat Point (PCH),
 Wildcat Point-LP (PCH), BayTrail (SOC), Sunrise Point-H (PCH),
 Sunrise Point-LP (PCH), DNV (SOC), Broxton (SOC),
 Lewisburg (PCH).
endef

$(eval $(call KernelPackage,i2c-i801))


I2C_MUX_MODULES:= \
  CONFIG_I2C_MUX:drivers/i2c/i2c-mux

define KernelPackage/i2c-mux
  $(call i2c_defaults,$(I2C_MUX_MODULES),51)
  TITLE:=I2C bus multiplexing support
  DEPENDS:=+kmod-i2c-core
endef

define KernelPackage/i2c-mux/description
 Kernel modules for I2C bus multiplexing support
endef

$(eval $(call KernelPackage,i2c-mux))

I2C_MUX_GPIO_MODULES:= \
  CONFIG_I2C_MUX_GPIO:drivers/i2c/muxes/i2c-mux-gpio

define KernelPackage/i2c-mux-gpio
  $(call i2c_defaults,$(I2C_MUX_GPIO_MODULES),51)
  TITLE:=GPIO-based I2C mux/switches
  DEPENDS:=+kmod-i2c-mux
endef

define KernelPackage/i2c-mux-gpio/description
 Kernel modules for GENERIC_GPIO I2C bus mux/switching devices
endef

$(eval $(call KernelPackage,i2c-mux-gpio))


I2C_MUX_REG_MODULES:= \
  CONFIG_I2C_MUX_REG:drivers/i2c/muxes/i2c-mux-reg

define KernelPackage/i2c-mux-reg
  $(call i2c_defaults,$(I2C_MUX_REG_MODULES),51)
  TITLE:=Register-based I2C mux/switches
  DEPENDS:=+kmod-i2c-mux
endef

define KernelPackage/i2c-mux-reg/description
 Kernel modules for register-based I2C bus mux/switching devices
endef

$(eval $(call KernelPackage,i2c-mux-reg))


I2C_MUX_PCA9541_MODULES:= \
  CONFIG_I2C_MUX_PCA9541:drivers/i2c/muxes/i2c-mux-pca9541

define KernelPackage/i2c-mux-pca9541
  $(call i2c_defaults,$(I2C_MUX_PCA9541_MODULES),51)
  TITLE:=Philips PCA9541 I2C mux/switches
  DEPENDS:=+kmod-i2c-mux
endef

define KernelPackage/i2c-mux-pca9541/description
 Kernel modules for PCA9541 I2C bus mux/switching devices
endef

$(eval $(call KernelPackage,i2c-mux-pca9541))

I2C_MUX_PCA954x_MODULES:= \
  CONFIG_I2C_MUX_PCA954x:drivers/i2c/muxes/i2c-mux-pca954x

define KernelPackage/i2c-mux-pca954x
  $(call i2c_defaults,$(I2C_MUX_PCA954x_MODULES),51)
  TITLE:=Philips PCA954x I2C mux/switches
  DEPENDS:=+kmod-i2c-mux
endef

define KernelPackage/i2c-mux-pca954x/description
 Kernel modules for PCA954x I2C bus mux/switching devices
endef

$(eval $(call KernelPackage,i2c-mux-pca954x))


I2C_PIIX4_MODULES:= \
  CONFIG_I2C_PIIX4:drivers/i2c/busses/i2c-piix4

define KernelPackage/i2c-piix4
  $(call i2c_defaults,$(I2C_PIIX4_MODULES),59)
  TITLE:=Intel PIIX4 and compatible I2C interfaces
  DEPENDS:=@PCI_SUPPORT @TARGET_x86 +kmod-i2c-core
endef

define KernelPackage/i2c-piix4/description
 Support for the Intel PIIX4 family of mainboard I2C interfaces,
 specifically Intel PIIX4, Intel 440MX, ATI IXP200, ATI IXP300,
 ATI IXP400, ATI SB600, ATI SB700/SP5100, ATI SB800, AMD Hudson-2,
 AMD ML, AMD CZ, Serverworks OSB4, Serverworks CSB5,
 Serverworks CSB6, Serverworks HT-1000, Serverworks HT-1100 and
 SMSC Victory66.
endef

$(eval $(call KernelPackage,i2c-piix4))


I2C_PXA_MODULES:= \
  CONFIG_I2C_PXA:drivers/i2c/busses/i2c-pxa

define KernelPackage/i2c-pxa
  $(call i2c_defaults,$(I2C_PXA_MODULES),50)
  TITLE:=Intel PXA I2C bus driver
  DEPENDS:=+kmod-i2c-core
endef

define KernelPackage/i2c-pxa/description
  Kernel module for Intel PXA2XX I2C adapter
endef

$(eval $(call KernelPackage,i2c-pxa))


I2C_SMBUS_MODULES:= \
  CONFIG_I2C_SMBUS:drivers/i2c/i2c-smbus

define KernelPackage/i2c-smbus
  $(call i2c_defaults,$(I2C_SMBUS_MODULES),58)
  TITLE:=SMBus-specific protocols helper
  DEPENDS:=+kmod-i2c-core
endef

define KernelPackage/i2c-smbus/description
 Support for the SMBus extensions to the I2C specification.
endef

$(eval $(call KernelPackage,i2c-smbus))



I2C_TINY_USB_MODULES:= \
  CONFIG_I2C_TINY_USB:drivers/i2c/busses/i2c-tiny-usb

define KernelPackage/i2c-tiny-usb
  $(call i2c_defaults,$(I2C_TINY_USB_MODULES),59)
  TITLE:=I2C Tiny USB adaptor
  DEPENDS:=@USB_SUPPORT +kmod-i2c-core +kmod-usb-core
endef

define KernelPackage/i2c-tiny-usb/description
 Kernel module for the I2C Tiny USB adaptor developed
 by Till Harbaum (http://www.harbaum.org/till/i2c_tiny_usb)
endef

$(eval $(call KernelPackage,i2c-tiny-usb))


