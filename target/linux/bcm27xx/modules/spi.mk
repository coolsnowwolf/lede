# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

define KernelPackage/spi-bcm2835
  SUBMENU:=$(SPI_MENU)
  TITLE:=BCM2835 SPI controller driver
  KCONFIG:=\
    CONFIG_SPI=y \
    CONFIG_SPI_BCM2835 \
    CONFIG_SPI_MASTER=y
  FILES:=$(LINUX_DIR)/drivers/spi/spi-bcm2835.ko
  AUTOLOAD:=$(call AutoLoad,89,spi-bcm2835)
  DEPENDS:=@TARGET_bcm27xx
endef

define KernelPackage/spi-bcm2835/description
  This package contains the Broadcom 2835 SPI master controller driver
endef

$(eval $(call KernelPackage,spi-bcm2835))


define KernelPackage/spi-bcm2835-aux
  SUBMENU:=$(SPI_MENU)
  TITLE:=BCM2835 Aux SPI controller driver
  KCONFIG:=\
    CONFIG_SPI=y \
    CONFIG_SPI_BCM2835AUX \
    CONFIG_SPI_MASTER=y
  FILES:=$(LINUX_DIR)/drivers/spi/spi-bcm2835aux.ko
  AUTOLOAD:=$(call AutoLoad,89,spi-bcm2835aux)
  DEPENDS:=@TARGET_bcm27xx
endef

define KernelPackage/spi-bcm2835-aux/description
  This package contains the Broadcom 2835 Aux SPI master controller driver
endef

$(eval $(call KernelPackage,spi-bcm2835-aux))
