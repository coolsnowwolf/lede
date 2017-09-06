ARCH:=mips
SUBTARGET:=xway_legacy
BOARDNAME:=XWAY Legacy
FEATURES:=squashfs atm ramdisk small_flash
CPU_TYPE:=24kc

DEFAULT_PACKAGES+=kmod-leds-gpio kmod-gpio-button-hotplug swconfig

define Target/Description
	Lantiq XWAY Legacy for old boards with small flash
endef
