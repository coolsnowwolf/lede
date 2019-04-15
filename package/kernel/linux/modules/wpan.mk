#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

WPAN_MENU:=WPAN 802.15.4 Support

define KernelPackage/ieee802154
  SUBMENU:=$(WPAN_MENU)
  TITLE:=IEEE-802.15.4 support
  DEPENDS:=@!LINUX_3_18
  KCONFIG:= \
	CONFIG_IEEE802154 \
	CONFIG_IEEE802154_SOCKET=y \
	CONFIG_IEEE802154_NL802154_EXPERIMENTAL=n
  FILES:= \
	$(LINUX_DIR)/net/ieee802154/ieee802154.ko \
	$(LINUX_DIR)/net/ieee802154/ieee802154_socket.ko@ge4.0
  AUTOLOAD:=$(call AutoLoad,90,ieee802154 ieee802154_socket)
endef

define KernelPackage/ieee802154/description
  IEEE Std 802.15.4 defines a low data rate, low power and low
  complexity short range wireless personal area networks. It was
  designed to organise networks of sensors, switches, etc automation
  devices. Maximum allowed data rate is 250 kb/s and typical personal
  operating space around 10m.
endef

$(eval $(call KernelPackage,ieee802154))

define KernelPackage/mac802154
  SUBMENU:=$(WPAN_MENU)
  TITLE:=MAC-802.15.4 support
  DEPENDS:=+kmod-ieee802154 +kmod-crypto-aead +kmod-lib-crc-ccitt @!LINUX_3_18
  KCONFIG:= \
	CONFIG_MAC802154 \
	CONFIG_IEEE802154_DRIVERS=y
  FILES:=$(LINUX_DIR)/net/mac802154/mac802154.ko
  AUTOLOAD:=$(call AutoLoad,91,mac802154)
endef

define KernelPackage/mac802154/description
  This option enables the hardware independent IEEE 802.15.4
  networking stack for SoftMAC devices (the ones implementing
  only PHY level of IEEE 802.15.4 standard).

  Note: this implementation is neither certified, nor feature
  complete! Compatibility with other implementations hasn't
  been tested yet!
endef

$(eval $(call KernelPackage,mac802154))

define KernelPackage/fakelb
  SUBMENU:=$(WPAN_MENU)
  TITLE:=Fake LR-WPAN driver
  DEPENDS:=+kmod-mac802154 @!LINUX_3_18
  KCONFIG:=CONFIG_IEEE802154_FAKELB
  FILES:=$(LINUX_DIR)/drivers/net/ieee802154/fakelb.ko
  AUTOLOAD:=$(call AutoLoad,92,fakelb)
endef

define KernelPackage/fakelb/description
  Say Y here to enable the fake driver that can emulate a net
  of several interconnected radio devices.
endef

$(eval $(call KernelPackage,fakelb))

define KernelPackage/at86rf230
  SUBMENU:=$(WPAN_MENU)
  TITLE:=AT86RF230 transceiver driver
  DEPENDS:=+kmod-mac802154 +kmod-regmap-spi
  KCONFIG:=CONFIG_IEEE802154_AT86RF230 \
	CONFIG_IEEE802154_AT86RF230_DEBUGFS=n \
	CONFIG_SPI=y \
	CONFIG_SPI_MASTER=y
  FILES:=$(LINUX_DIR)/drivers/net/ieee802154/at86rf230.ko
endef

$(eval $(call KernelPackage,at86rf230))

define KernelPackage/mrf24j40
  SUBMENU:=$(WPAN_MENU)
  TITLE:=MRF24J40 transceiver driver
  DEPENDS:=+kmod-mac802154 +kmod-regmap-spi
  KCONFIG:=CONFIG_IEEE802154_MRF24J40 \
	CONFIG_SPI=y \
	CONFIG_SPI_MASTER=y
  FILES:=$(LINUX_DIR)/drivers/net/ieee802154/mrf24j40.ko
endef

$(eval $(call KernelPackage,mrf24j40))

define KernelPackage/cc2520
  SUBMENU:=$(WPAN_MENU)
  TITLE:=CC2520 transceiver driver
  DEPENDS:=+kmod-mac802154
  KCONFIG:=CONFIG_IEEE802154_CC2520 \
	CONFIG_SPI=y \
	CONFIG_SPI_MASTER=y
  FILES:=$(LINUX_DIR)/drivers/net/ieee802154/cc2520.ko
endef

$(eval $(call KernelPackage,cc2520))

define KernelPackage/ieee802154_6lowpan
  SUBMENU:=$(WPAN_MENU)
  TITLE:= 6LoWPAN support over IEEE-802.15.4
  DEPENDS:=@!LINUX_3_18 +kmod-6lowpan +kmod-ieee802154
  KCONFIG:=CONFIG_IEEE802154_6LOWPAN
  FILES:= \
	$(LINUX_DIR)/net/ieee802154/6lowpan/ieee802154_6lowpan.ko@ge4.0 \
	$(LINUX_DIR)/net/ieee802154/ieee802154_6lowpan.ko@lt4.0
  AUTOLOAD:=$(call AutoLoad,91,ieee802154_6lowpan)
endef

define KernelPackage/ieee802154_6lowpan/description
 IPv6 compression over IEEE 802.15.4
endef

$(eval $(call KernelPackage,ieee802154_6lowpan))
